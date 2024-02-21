#include <iostream>
#include <fstream>
#include "json_reader.hpp"


int main(int argc, char **argv) {
    if (argc != 2) {
        std::cerr << "Wrong number of arguments. Exactly one argument is required" << std::endl;
        std::exit(0);
    }
    std::ifstream ifs(argv[1]);
    nlohmann::json jsonParams = nlohmann::json::parse(ifs);

    PnlMat *correlation;
    jsonParams.at("Correlations").get_to(correlation);
    pnl_mat_print(correlation);

    std::string domesticCurrencyId;
    jsonParams.at("DomesticCurrencyId").get_to(domesticCurrencyId);
    int assetNb = jsonParams.at("Assets").size();
    std::cout << "Number of assets " << assetNb << std::endl;

    std::cout << "-- currencies" << std::endl;
    auto jsonCurrencies = jsonParams.at("Currencies");
    for (auto jsonCurrency : jsonCurrencies) {
        std::string currencyId(jsonCurrency.at("id").get<std::string>());
        double rf = jsonCurrency.at("InterestRate").get<double>();
        double realVolatility = jsonCurrency.at("Volatility").get<double>();
        std::cout << "interest rate " << rf << std::endl;
        std::cout << "real volatility " << realVolatility << std::endl;
    }

    std::cout << "-- assets" << std::endl;
    auto jsonAssets = jsonParams.at("Assets");
    for (auto jsonAsset : jsonAssets) {
        std::string currencyId(jsonAsset.at("CurrencyId").get<std::string>());
        double realVolatility = jsonAsset.at("Volatility").get<double>();
        std::cout << "currency " << currencyId << std::endl;
        std::cout << "real volatility " << realVolatility << std::endl;
    }

    int numberOfDaysPerYear = jsonParams.at("NumberOfDaysInOneYear").get<int>();
    double maturity = jsonParams.at("Option").at("MaturityInDays").get<int>() / double (numberOfDaysPerYear);
    std::string label = jsonParams.at("Option").at("Type").get<std::string>();

    pnl_mat_free(&correlation);
    std::exit(0);
}