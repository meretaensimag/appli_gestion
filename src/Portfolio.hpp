#pragma once

#include <list>
#include "MonteCarlo.hpp"
#include "TimeGrid.hpp"
#include "Position.hpp"

class Portfolio {
public:
    MonteCarlo &monteCarlo;
    std::list<Position> positions;

    Portfolio(MonteCarlo &monteCarlo);
    ~Portfolio();
};
