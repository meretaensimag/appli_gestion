#include "MonteCarlo.hpp"
#include <iostream>

MonteCarlo :: MonteCarlo(Option* opt, GlobalModel* mod, double fdStep, int sampleNb) {
    option_ = opt;
    model_ = mod;
    nbAssets_ = model_->assets_.size() + model_->currencies_.size();
    fdStep_ = fdStep;
    nbSamples_ = sampleNb;
    rng_= pnl_rng_create(PNL_RNG_MERSENNE);
    pnl_rng_sseed(rng_, time(NULL));

}

void MonteCarlo::priceAndDelta(PnlMat *pastData, int currentEvalDate, double &computedPrice, double &priceVariance, PnlVect *&assetDeltas, PnlVect *&deltaVariances) {
    computedPrice = 0.0;
    priceVariance = 0.0;
    assetDeltas = pnl_vect_create_from_zero(nbAssets_);
    deltaVariances = pnl_vect_create_from_zero(nbAssets_);
    double totalPayoff = 0.0;
    double totalPayoffSquared = 0.0;
    PnlMat *simulatedPath = pnl_mat_create(model_->timeGrid_->nbTimeSteps_, nbAssets_);
    double singleDelta = 0;
    double assetPayoff = 0;
    double assetPayoffUp = 0;
    double assetPayoffDown = 0;
    //boucle sur les simulations
    for (int sampleIndex = 0; sampleIndex < nbSamples_; sampleIndex++) {
        model_ -> sample(pastData, simulatedPath, currentEvalDate, rng_);
        assetPayoff = option_->payoff(simulatedPath);
        totalPayoff += assetPayoff;
        totalPayoffSquared += assetPayoff * assetPayoff;
        //boucle sur les actifs
        for (int assetIndex = 0; assetIndex < nbAssets_; assetIndex++) {
            model_->shiftAsset(simulatedPath, simulatedPath, assetIndex, fdStep_, currentEvalDate);
            assetPayoffUp = option_->payoff(simulatedPath);
            model_->shiftAsset(simulatedPath, simulatedPath, assetIndex, (-2 * fdStep_) / (1 + fdStep_), currentEvalDate);
            assetPayoffDown = option_->payoff(simulatedPath);
            //calcul du delta qui est la dérivée partielle du prix de l'option par rapport au prix de l'actif
            singleDelta = assetPayoffUp - assetPayoffDown;
            model_->shiftAsset(simulatedPath, simulatedPath, assetIndex, fdStep_ / (1-fdStep_), currentEvalDate);
            //mis a jour du vecteur des deltas
            LET(assetDeltas, assetIndex) = GET(assetDeltas, assetIndex) + singleDelta;
            LET(deltaVariances, assetIndex) = GET(deltaVariances, assetIndex) + singleDelta*singleDelta;
        }
    }
    //calcul du prix de l'option
    //int numberOfDaysInOneYear = option_->numberOfDaysInOneYear_;
    //double maturityInYears = option_->maturity_ / (double) numberOfDaysInOneYear;
    int numberOfDaysInOneYear = 252;
    double maturityInYears = model_->timeGrid_->maturity_ / (double) numberOfDaysInOneYear;
   
    double domesticInterestRate = option_->domesticInterestRate_;
    double currentEvalDateInYears = currentEvalDate/(double) numberOfDaysInOneYear;
    double discountFactor = exp(-domesticInterestRate * (maturityInYears - currentEvalDateInYears));
    totalPayoff /= nbSamples_;
    computedPrice = discountFactor * totalPayoff;
    priceVariance = std::sqrt(std::abs(totalPayoffSquared / nbSamples_ - computedPrice * computedPrice) / nbSamples_);
    computedPrice = discountFactor * computedPrice;
    //priceVariance = discountFactor  * priceVariance;
    //priceVariance = exp(-2*domesticInterestRate* (maturityInYears - currentEvalDateInYears)) * (totalPayoffSquared - computedPrice * computedPrice);
    //calcul du delta de l'option
    double avgDeltaFactor = discountFactor / (2 * fdStep_ * nbSamples_);
    double avgDeltaSquaredFactor = avgDeltaFactor * avgDeltaFactor * nbSamples_;
    double spotPrice = 0;
    double varianceAdjustment = 0;
    //boucle sur les actifs
    double delta_squared_value;
    double DeltaConst = discountFactor / (2 * fdStep_ * nbSamples_);
    double SquareDeltaConst = DeltaConst * DeltaConst * nbSamples_;
    for (int deltaIndex = 0; deltaIndex < nbAssets_; deltaIndex++) {
        spotPrice = pnl_mat_get(pastData, pastData->m - 1, deltaIndex);
        singleDelta = pnl_vect_get(assetDeltas, deltaIndex);
        LET(assetDeltas, deltaIndex) = exp(-domesticInterestRate * (maturityInYears - currentEvalDateInYears)) * (GET(assetDeltas, deltaIndex) / (nbSamples_ * (2 * fdStep_*spotPrice)));
        varianceAdjustment = pnl_vect_get(deltaVariances, deltaIndex) * (avgDeltaSquaredFactor / (spotPrice * spotPrice)) - std::pow(singleDelta / (2 * fdStep_ * nbSamples_ * spotPrice), 2.0);
        pnl_vect_set(deltaVariances, deltaIndex, sqrt(abs(varianceAdjustment) / nbSamples_));
//        delta_squared_value = GET(deltaVariances, deltaIndex) * SquareDeltaConst / (spotPrice * spotPrice) - (singleDelta / discountFactor) * (singleDelta / discountFactor);
//        LET(deltaVariances, deltaIndex) = std::sqrt(std::abs(delta_squared_value / nbSamples_));
    }
    pnl_mat_free(&simulatedPath);
}

MonteCarlo::~MonteCarlo(){
    pnl_rng_free(&rng_);
    delete(option_);
    delete(model_);
}