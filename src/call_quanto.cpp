//
// Created by ensimag on 03/02/24.
//

#include <utility>
#include <algorithm>
#include <iostream>

#include "Option.hpp"

call_quanto :: call_quanto(std::vector<int> assetCurrencyMapping,
                               std::vector<double> foreignInterestRates,
                               double domesticInterestRate,
                               double strike) {
    this->assetCurrencyMapping_ = std::move(assetCurrencyMapping);
    this->foreignInterestRates_ = std::move(foreignInterestRates);
    this->domesticInterestRate_ = domesticInterestRate;
    this->strike_ = strike;
}

double call_quanto :: payoff(const PnlMat* path)
{
    double SX =  pnl_mat_get(path, path->m-1, 0);
    double value = (SX > strike_) ? (SX - strike_) : 0;
    return (SX > strike_) ? (SX - strike_) : 0;
}