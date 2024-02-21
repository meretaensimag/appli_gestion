#pragma once

#include "GlobalModel.hpp"

class MonteCarlo {
public:
    Option* option_;
    GlobalModel* model_;
    PnlRng* rng_;
    double fdStep_;
    int nbSamples_;
    int nbAssets_;


    MonteCarlo(Option* opt, GlobalModel* mod, double fdStep, int sampleNb);

    void priceAndDelta(
            PnlMat *pastData,
            int currentEvalDate,
            double &computedPrice,
            double &priceVariance,
            PnlVect *&assetDeltas,
            PnlVect *&deltaVariances);
    ~MonteCarlo();
};
