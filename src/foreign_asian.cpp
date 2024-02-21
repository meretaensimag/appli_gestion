//
// Created by ensimag on 03/02/24.
//

#include <utility>

#include "Option.hpp"

foreign_asian :: foreign_asian(std::vector<int> assetCurrencyMapping,std::vector<double> foreignInterestRates,
                               double domesticInterestRate,
                               double strike) {
    this->assetCurrencyMapping_ = std::move(assetCurrencyMapping);
    this->foreignInterestRates_ = std::move(foreignInterestRates);
    this->domesticInterestRate_ = domesticInterestRate;
    this->strike_ = strike;
}

double foreign_asian :: payoff(const PnlMat* path)
{
    int N = path->m;
    double somme = 0.0;
    for (int i = 0; i < N; i++)
    {
        somme += pnl_mat_get(path, i, 1);
    }
    somme = (1.0 / (double)N) * somme;
    return std::max(somme - pnl_mat_get(path, path->m-1, 0), 0.0);
}