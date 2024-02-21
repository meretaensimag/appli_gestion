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
    double strike_;

    virtual double payoff(const PnlMat *path) = 0;
    virtual ~Option() = default;
};

class call_quanto : public Option
{
public:
    call_quanto(std::vector<int> assetCurrencyMapping,std::vector<double> foreignInterestRates, double domesticInterestRate, double strike);
    double payoff(const PnlMat* path) override;
};

class quanto_exchange : public Option
{
public:
    quanto_exchange(std::vector<int> assetCurrencyMapping,std::vector<double> foreignInterestRates, double domesticInterestRate, double strike);
    double payoff(const PnlMat* path) override;
    ~quanto_exchange(){}
};

class call_currency : public Option
{
public:
    call_currency(std::vector<int> assetCurrencyMapping,std::vector<double> foreignInterestRates, double domesticInterestRate, double strike);
    double payoff(const PnlMat* path) override;
    ~call_currency(){}
};
class foreign_asian : public Option
{
public:
    foreign_asian(std::vector<int> assetCurrencyMapping,std::vector<double> foreignInterestRates, double domesticInterestRate, double strike);
    double payoff(const PnlMat* path) override;
    ~foreign_asian(){}
};

class foreign_perf_basket : public Option
{
public:
    TimeGrid *timeGrid_;
    foreign_perf_basket(std::vector<int> assetCurrencyMapping,std::vector<double> foreignInterestRates,TimeGrid *timeGrid, double domesticInterestRate, double strike);
    double payoff(const PnlMat* path) override;
    ~foreign_perf_basket(){}
};