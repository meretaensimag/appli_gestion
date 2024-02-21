#include "Currency.hpp"

Currency :: Currency(double drift,
                                 PnlVect *volatilityVector,
                                 double interestRate,
                                 double domesticInterestRate) : RiskyAsset(drift, volatilityVector, domesticInterestRate){
    interestRate_ = interestRate;
}
