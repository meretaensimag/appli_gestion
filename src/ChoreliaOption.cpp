#include "Option.hpp"
#include <stdexcept>
#include <cmath>
#include <iostream>
#include "fstream"

ChoreliaOption :: ChoreliaOption(std::vector<int> assetCurrencyMapping,
                                 std::vector<double> foreignInterestRates,
                                 TimeGrid *timeGrid,
                                 double domesticInterestRate,
                                 double referential_amount) {
    this->assetCurrencyMapping_ = std::move(assetCurrencyMapping);
    this->foreignInterestRates_ = std::move(foreignInterestRates);
    this->domesticInterestRate_ = domesticInterestRate;
    this->timeGrid_ = timeGrid;
    this->referential_amount_ = referential_amount;
}


double ChoreliaOption :: adjust_rate(double rate) {
    // La performance est limitee à la baisse à -15%
    if (rate < -0.15) {
        return -0.15;
    }
    return rate;
}

// Calcule la moyenne des taux de rendement après exclusion du meilleur et du pire
double ChoreliaOption :: calculate_average_rate(PnlVect *rates, const PnlMat *path, int index) {

    for (int j = 0; j < 5; ++j) {
        double spot_at_tmo;
        double spot_at_t;
        if(j==0||j==1){
            spot_at_tmo = pnl_mat_get(path, index-1, j);
            spot_at_t = pnl_mat_get(path, index, j);
        }
        else{
//            spot_at_0 = pnl_mat_get(path, 0, j)*exp(foreignInterestRates_[j-2]*(timeGrid_->dateList_[0]-timeGrid_->maturity_)/(double)365)/(double)pnl_mat_get(path,0 , j+3);
//            spot_at_t = pnl_mat_get(path, index, j)*exp(foreignInterestRates_[j-2]*(timeGrid_->dateList_[index]-timeGrid_->maturity_)/(double)365)/(double)pnl_mat_get(path,index , j+3);
            spot_at_tmo = pnl_mat_get(path, index-1, j)/(double)pnl_mat_get(path,index-1 , j+3);
            spot_at_t = pnl_mat_get(path, index, j)/(double)pnl_mat_get(path,index , j+3);
            spot_at_tmo*= exp(foreignInterestRates_[j-2]*(timeGrid_->dateList_[index-1]-timeGrid_->maturity_)/(double)365);
            spot_at_t *= exp(foreignInterestRates_[j-2]*(timeGrid_->dateList_[index]-timeGrid_->maturity_)/(double)365);
        }
        double rate = (spot_at_t - spot_at_tmo) / (double)spot_at_tmo;
        pnl_vect_set(rates, j, rate);
    }

    // Trier le vecteur pour pouvoir exclure la meilleure et la pire rentabilite
    pnl_vect_qsort(rates, 'd');
    // Calculer la moyenne en excluant le premier et le dernier élément
    double sum = pnl_vect_get(rates, 1)+pnl_vect_get(rates, 2)+pnl_vect_get(rates, 3);
    return adjust_rate(sum /(double) 3); // Retourner la moyenne des 3 rentabilites intermediaires
}

double ChoreliaOption :: payoff(int currentDate, const PnlMat *path) {
    //pnl_mat_print(path);
    double final_perf = 1.0; // Cumul de la performance annuelle
    double payoff = 0.;
    double dividend = 0.0;   // Dividende à payer
    PnlVect *rates = pnl_vect_create_from_zero(5);
    for (int i = 1; i < path->m-1; i++) {
        double basket_perf = calculate_average_rate(rates, path, i);

        final_perf *= (basket_perf+1);

        if(pnl_vect_get(rates,2) >= 0){
            dividend = 25 * pnl_vect_get(rates,2); // La troisième meilleure performance
            //std::cout << " ana hnaaaaaaaaaaaaaaaaaa dividend " << dividend   << std::endl;
        }
        else{
            dividend = 0.0;
        }
        //std::cout << " ana hnaaaaaaaaaaaaaaaaaa dividend " << dividend   << std::endl;
        payoff += dividend * std::exp(domesticInterestRate_ *(timeGrid_->maturity_ - timeGrid_->dateList_[i])/(double)365);
        rates = pnl_vect_create_from_zero(5);
    }
    double basket_perf = calculate_average_rate(rates, path, path->m - 1);
    final_perf *= (basket_perf+1);
    payoff += referential_amount_*(1+0.25*(final_perf-1));
    //std::cout << " ana hnaaaaaaaaaaaaaaaaaa referential_amount_ " << referential_amount_*(1+0.25*(final_perf-1))  << std::endl;
    return payoff;
}