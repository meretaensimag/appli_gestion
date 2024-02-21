#pragma once
#include "IRebalancingOracle.hpp"

class FixedTimesOracle : public IRebalacingOracle {

public:
    FixedTimesOracle(TimeGrid* timeGrid, int period); 
    bool rebalancingDate(int date) override; 
    int period;
    int compteur = 0;
};