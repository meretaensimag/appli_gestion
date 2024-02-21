#include "RiskyAsset.hpp"

RiskyAsset :: RiskyAsset(double drift, PnlVect* volatilityVector, double domesticInterestRate){
    drift_ = drift;
    volatilityVector_ = volatilityVector;
    domesticInterestRate_ = domesticInterestRate;
};
