#include "IRebalancingOracle.hpp"

class GridOracle : public IRebalacingOracle {
    std::vector<int> RebalancingTimeList_;
    int compteur = 0;

public:

    bool rebalancingDate(int date) override {
        RebalancingTimeList_ = this->timeGrid->rebalancingDates_;
        auto it = std::find(RebalancingTimeList_.begin(), RebalancingTimeList_.end(), date);
        return it != RebalancingTimeList_.end();
    }
};
