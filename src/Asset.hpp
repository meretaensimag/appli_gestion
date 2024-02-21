#pragma once
#include "RiskyAsset.hpp"

class Asset : public RiskyAsset {
public:
    Asset(double domesticInterestRate, double drift, PnlVect *volatilityVector);

};
