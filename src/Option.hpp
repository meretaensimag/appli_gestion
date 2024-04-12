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

    virtual double payoff(const PnlMat *path) = 0;
    virtual ~Option() = default;
};

class ChoreliaOption : public Option {
private:
    TimeGrid *timeGrid_;

public:
    ChoreliaOption (std::vector<int> assetCurrencyMapping,std::vector<double> foreignInterestRates,TimeGrid *timeGrid, double domesticInterestRate, double referential_amount);

    static double adjust_rate(double rate);
    static double calculate_average_rate(const PnlMat *path, int index);
    double payoff(const PnlMat *path) override;
    ~ChoreliaOption() override= default;
};