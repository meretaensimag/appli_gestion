#pragma once
#include "json_reader.hpp"
#include "TimeGrid.hpp"

class IRebalacingOracle {
public:
    TimeGrid* timeGrid;
    virtual bool rebalancingDate(int date) = 0;
    virtual ~IRebalacingOracle() {}
};