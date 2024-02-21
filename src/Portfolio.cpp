#include "Portfolio.hpp"


void to_json(nlohmann::json &j, const Position &position) {
    j["date"] = position.date;
    j["value"] = position.portfolioValue;
    j["price"] = position.price;
    j["priceStdDev"] = position.priceStdDev;
    j["deltas"] = position.deltas;
    j["deltasStdDev"] = position.deltasStdDev;
}

Portfolio::Portfolio(MonteCarlo &monteCarlo)
    : monteCarlo(monteCarlo) {
    //int numberOfDaysPerYear = jsonParams.at("NumberOfDaysInOneYear").get<int>();
    //int maturityInDays = jsonParams.at("Option").at("MaturityInDays").get<int>();

}

Portfolio::~Portfolio() {
}

