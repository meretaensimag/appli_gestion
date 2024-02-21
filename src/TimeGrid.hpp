#pragma once
#include "json_reader.hpp"

class TimeGrid
{
public:
    std::vector<int> dateList_;
    int maturity_;
    int nbTimeSteps_;
    std::vector<int> rebalancingDates_;

    TimeGrid(nlohmann::json &jsonParams);
    
    bool isMonitoringDate(int time);

};