#pragma once

#include "pnl/pnl_vector.h"
#include "pnl/pnl_matrix.h"
#include "TimeGrid.hpp"
#include <vector>

/// \brief Classe Option abstraite
class Option
{
public:
    std::vector<int> assetCurrencyMapping_;
    std::vector<double> foreignInterestRates_;
    double domesticInterestRate_;
    double referential_amount_;

    virtual double payoff(int currentDate, const PnlMat *path) = 0;
    virtual ~Option() = default;
};

class ChoreliaOption : public Option {
private:
    TimeGrid *timeGrid_;

public:
    ChoreliaOption (std::vector<int> assetCurrencyMapping,std::vector<double> foreignInterestRates,TimeGrid *timeGrid, double domesticInterestRate, double referential_amount);

    double adjust_rate(double rate);
    double calculate_average_rate(PnlVect *rates, const PnlMat *path, int index);
    double payoff(int currentDate, const PnlMat *path) override;
    ~ChoreliaOption() override= default;
};