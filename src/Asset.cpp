#include "Asset.hpp"

Asset ::Asset(double domesticInterestRate, double drift, PnlVect *volatilityVector) : RiskyAsset(drift, volatilityVector, domesticInterestRate) {
    volatilityVector_ = volatilityVector;
}


