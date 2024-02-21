#include <utility>
#include <cmath>
#include "Option.hpp"

call_currency :: call_currency(std::vector<int> assetCurrencyMapping,
std::vector<double> foreignInterestRates,
double domesticInterestRate,
double strike) {
this->assetCurrencyMapping_ = std::move(assetCurrencyMapping);
this->foreignInterestRates_ = std::move(foreignInterestRates);
this->domesticInterestRate_ = domesticInterestRate;
this->strike_ = strike;

}

double call_currency :: payoff(const PnlMat* path)
{
    double value =  pnl_mat_get(path, path->m-1,0);
    return (value > strike_) ? (value - strike_) : 0;
}