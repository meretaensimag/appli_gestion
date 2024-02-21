#include "FixedTimesOracle.hpp"

FixedTimesOracle::FixedTimesOracle(TimeGrid* time_grid, int period) {
    this->timeGrid = time_grid; 
    this->period = period; 
}

bool FixedTimesOracle::rebalancingDate(int date) {
        compteur++;
        return (compteur % period == 0);

}; 
