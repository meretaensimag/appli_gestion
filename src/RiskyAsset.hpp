#pragma once

#include <pnl/pnl_vector.h>

class RiskyAsset {
public:
    double drift_;
    PnlVect *volatilityVector_;
    double domesticInterestRate_;

    RiskyAsset(double drift, PnlVect* volatilityVector, double domesticInterestRate);
};