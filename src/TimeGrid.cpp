#include "TimeGrid.hpp"

TimeGrid::TimeGrid(nlohmann::json &jsonParams){
    maturity_ = jsonParams.at("Option").at("MaturityInDays").get<int>();
    std::string label = jsonParams.at("Option").at("FixingDatesInDays").at("Type").get<std::string>();
    //std::string label2 = jsonParams.at("PortfolioRebalancingOracleDescription").at("Type").get<std::string>();


    if (label == "Grid") {
        dateList_ = jsonParams.at("Option").at("FixingDatesInDays").at("DatesInDays").get<std::vector<int>>();
    }
    else if (label == "Fixed") {
        int period = jsonParams.at("Option").at("FixingDatesInDays").at("Period").get<int>();
        int currentDate = 0;
        while (currentDate <= maturity_) {
            dateList_.push_back(currentDate);
            currentDate += period;
        }
    }

//    if (label2 == "Grid") {
//        rebalancingDates_ = jsonParams.at("PortfolioRebalancingOracleDescription").at("DatesInDays").get<std::vector<int>>();
//    }

    nbTimeSteps_ = dateList_.size();
}

bool TimeGrid::isMonitoringDate(int date) {
    for (int elem: dateList_) {
        if (elem == date) {
            return true;
        }
    }
    return false;
}

int TimeGrid::getIndex(int t) {
    if (isMonitoringDate(t)) {
        for (int i = 0; i < dateList_.size(); i++) {
            if (dateList_[i] == t) {
                return i;
            }
        }
    }
    return dateList_[dateList_.size() - 1];
}


int TimeGrid::getNextMonitoringDateIndex(int t) {
    for (int i = 0; i < dateList_.size(); i++) {
        if (dateList_[i] > t) {
            return i;
        }
    }
    return dateList_[dateList_.size() - 1];
}
