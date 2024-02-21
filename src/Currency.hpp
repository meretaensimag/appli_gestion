#pragma once
#include "RiskyAsset.hpp"

class Currency : public RiskyAsset {
public:
    double interestRate_;
public:
    Currency(double drift,
             PnlVect *volatilityVector,
             double interestRate,
             double domesticInterestRate);
};