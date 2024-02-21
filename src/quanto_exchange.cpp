//
// Created by ensimag on 03/02/24.
//

#include <utility>
#include <algorithm>

#include "Option.hpp"

quanto_exchange :: quanto_exchange(std::vector<int> assetCurrencyMapping,
                                   std::vector<double> foreignInterestRates,
                                   double domesticInterestRate,
                                   double strike) {
    this->assetCurrencyMapping_ = std::move(assetCurrencyMapping);
    this->foreignInterestRates_ = std::move(foreignInterestRates);
    this->domesticInterestRate_ = domesticInterestRate;
    this->strike_ = strike;
}

double quanto_exchange :: payoff(const PnlMat* path)
{
    double value =  pnl_mat_get(path, path->m-1, 0)- pnl_mat_get(path, path->m-1, 1);
    return (value > strike_) ? (value - strike_) : 0;
}