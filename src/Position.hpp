#pragma once

#include "pnl/pnl_vector.h"
#include "json_reader.hpp"
#include <iostream>

class Position {
public:
    int date;
    PnlVect *deltas;
    PnlVect *deltasStdDev;
    double price;
    double priceStdDev;
    double portfolioValue;

    Position(int date, double price, double priceStdDev, PnlVect* deltas, PnlVect* deltasStdDev, double portfolioValue);
    friend void to_json(nlohmann::json &j, const Position &positions);
    void print() const;
};