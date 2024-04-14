#include "GlobalModel.hpp"
#include "pnl/pnl_matrix.h"
#include "pnl/pnl_random.h"
#include <iostream>
using namespace std;


vector<RiskyAsset> GlobalModel::createRiskyVector(vector<Currency> currencies, vector<Asset> assets){
    vector<RiskyAsset> riskyVector;

    for (const Asset &asset : assets) {
        riskyVector.push_back(asset);
    }

    for (int i = 0; i < currencies.size(); i++) {
        riskyVector.push_back(currencies.at(i));
    }
    return riskyVector;

}

GlobalModel::GlobalModel(std::vector<Asset> assets, std::vector<Currency> currencies, TimeGrid* timeGrid,PnlMat* hedgingPast) {
    riskyAssets_ = createRiskyVector(currencies, assets);
    assets_ = assets;
    currencies_ = currencies;
    timeGrid_ = timeGrid;
    G_ = pnl_vect_create(assets.size() + currencies.size());
    spot_ = pnl_vect_create(assets.size() + currencies.size());
    vol_ = pnl_vect_create(assets.size() + currencies.size());
    timeGrid = timeGrid;
    hedgingPast = hedgingPast;
}
GlobalModel::~GlobalModel() {
    //pnl_mat_free(&correlation_matrice_);
    pnl_vect_free(&spot_);
    pnl_vect_free(&vol_);
    //delete(timeGrid_);
    currencies_.clear();
    assets_.clear();
}

int GlobalModel::getIndex(std::vector<int> dates, int time) {

    for (int i = 0; i < dates.size() ; i++){
        if (time < dates.at(i)) return (i);
    }
}
void GlobalModel::sample(PnlMat* past, PnlMat* path, int t, PnlRng* rng) {

    pnl_mat_set_subblock(path, past, 0,0);

    int size = past->n;
    vector<int> dates = timeGrid_->dateList_;

    int startingIndex = past->m-1;


    pnl_mat_get_row(spot_,past, past->m-1);

    double dt = (double)(dates.at(startingIndex)-t)/(double)365;
    pnl_vect_rng_normal(G_, assets_.size() + currencies_.size(), rng);
    stepSimulation(spot_, dt, size, G_);

    if(timeGrid_->isMonitoringDate(t) && t!= timeGrid_->dateList_[0] && t!= timeGrid_->maturity_){
        startingIndex = past->m;
    }

    pnl_mat_set_row(path, spot_, startingIndex);


    for (int index=startingIndex+1; index <dates.size(); index++) {
        dt = (double)(dates.at(index)-dates.at(index-1))/(double)365;
        pnl_vect_rng_normal(G_, assets_.size() + currencies_.size(), rng);
        stepSimulation(spot_, dt, size, G_);
        pnl_mat_set_row(path, spot_, index);
    }
}
void GlobalModel ::stepSimulation(PnlVect *current_spot, double dt, int size, PnlVect * G) {
    double prevValue;
    double currValue;
    for (int d = 0; d < size; d++) {
        vol_ = riskyAssets_.at(d).volatilityVector_;
        prevValue = GET(current_spot, d);
        double random_term = pnl_vect_scalar_prod(vol_, G);
        currValue = prevValue * exp((riskyAssets_.at(d ).drift_ - pnl_vect_scalar_prod(vol_, vol_)*0.5)* dt + sqrt(dt) * random_term);
        pnl_vect_set(current_spot, d, currValue);
    }
}


void GlobalModel::shiftAsset(PnlMat *shift_path, const PnlMat *path, int d, double fdStep, double currentDate) {
    int nextIndex;
    if(timeGrid_->isMonitoringDate(currentDate) && currentDate!=0){
        nextIndex= timeGrid_->getIndex(currentDate);
    }
    else{
        nextIndex= timeGrid_->getNextMonitoringDateIndex(currentDate);
    }
    for (int i = nextIndex; i < path->m; i++) {
        MLET(shift_path, i, d) =MGET(shift_path, i, d) *(1 + fdStep);
    }

//    int k = 1;
//    int size = timeGrid_->dateList_.size();
//    std::vector<int> dates = timeGrid_->dateList_;
//    while (k < size ) {
//        if (dates[k] >= currentDate) {
//            break;
//        }
//        k += 1;
//    }
//    for (int i = k; i < path->m; i++) {
//        MLET(shift_path, i, d) *= (1+fdStep);
//    }

}