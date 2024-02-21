#include "Position.hpp"

Position::Position(int date, double price, double priceStdDev, PnlVect* deltas, PnlVect* deltasStdDev, double portfolioValue)
    : date(date), price(price), priceStdDev(priceStdDev), portfolioValue(portfolioValue), deltas(deltas), deltasStdDev(deltasStdDev) {

}

void Position::print() const {
    nlohmann::json j = *this;
    std::cout << j.dump(4);
}