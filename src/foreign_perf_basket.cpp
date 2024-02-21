#include <utility>

#include "Option.hpp"
#include <algorithm> // Pour std::count

foreign_perf_basket :: foreign_perf_basket(std::vector<int> assetCurrencyMapping,
                                           std::vector<double> foreignInterestRates,
                                           TimeGrid *timeGrid,
                                           double domesticInterestRate,
                                           double strike) {
    this->assetCurrencyMapping_ = std::move(assetCurrencyMapping);
    this->foreignInterestRates_ = std::move(foreignInterestRates);
    this->domesticInterestRate_ = domesticInterestRate;
    this->timeGrid_ = timeGrid;
    this->strike_ = strike;
}

double foreign_perf_basket :: payoff(const PnlMat* path)
{
    std::vector<double> performances;
    int imax = 0;
    int idmax;
    int NbrDevises = assetCurrencyMapping_.back();
    double perf_max = 0.0;
    double perf;
    double t0 = timeGrid_->dateList_[0]/(double)252;
    double t1 = timeGrid_->dateList_[1]/(double)252;
    int n = std::count(assetCurrencyMapping_.begin(), assetCurrencyMapping_.end(), 0);
    for (int i = 0; i < NbrDevises; i++) {
        int ni = std::count(assetCurrencyMapping_.begin(), assetCurrencyMapping_.end(), i+1);
        double num = 0.0;
        double denum = 0.0;
        for (int l = 0; l < ni; l++) {
            num += pnl_mat_get(path,1 , n+l)*exp(foreignInterestRates_[i]*t1)/(double)pnl_mat_get(path,1 , (int)assetCurrencyMapping_.size()+i);
            denum += pnl_mat_get(path,0 , n+l)*exp(foreignInterestRates_[i]*t0)/(double)pnl_mat_get(path,0 , (int)assetCurrencyMapping_.size()+i);
        }
        perf = (exp(foreignInterestRates_[i]*t0)/exp(foreignInterestRates_[i]*t1))*(num/denum);
        if(perf>perf_max){
            perf_max = perf;
            imax = i+1;
            idmax = n;
        }
        n+= ni;
    }
    
    double maxForeignPerformance = 0.0;

    int nimax = (int)std::count(assetCurrencyMapping_.begin(), assetCurrencyMapping_.end(), imax);
    for(int i=0; i< nimax; i++){
        maxForeignPerformance += pnl_mat_get(path, path->m-1, idmax+i);
    }
    maxForeignPerformance = (1/(double)nimax)*maxForeignPerformance;

    double domValue = 0.0;
    int n0 = std::count(assetCurrencyMapping_.begin(), assetCurrencyMapping_.end(), 0);
    for(int i=0; i< n0; i++){
        domValue += pnl_mat_get(path, path->m-1, i);
    }
    domValue = (1/(double)n0)*domValue;
    double performanceDifference = maxForeignPerformance - domValue;

    return std::max(performanceDifference-strike_, 0.0);
}