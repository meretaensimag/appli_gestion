#pragma once

#include <nlohmann/json.hpp>
#include "json_reader.hpp"
#include <vector>
#include <utility> // Pour std::pair
#include <pnl/pnl_matrix.h>
#include <string>
//#include "RiskyAsset.hpp"
#include "Currency.hpp"
#include "Asset.hpp"
#include "TimeGrid.hpp"

class JsonParser {
public:
    nlohmann::json j;
    PnlMat* Correlation;
    explicit JsonParser(const std::string& filePath);

public:

    std::pair<std::vector<Currency>, std::vector<double>> parseCurrencies();

    std::pair<std::vector<Asset>, std::vector<int>> parseAssets(const std::vector<Currency>& currencies);
    void parsePast(PnlMat* marketData, TimeGrid *timeGrid, const std::vector<int>& assetCurrencyMapping, const std::vector<double>& foreignInterestRates);

    double parseDom(std::vector<Currency> currencies);

    void computeCorMatrix();

        // Méthode pour obtenir la valeur du strike
    double parseReferentialAmount();

    ~JsonParser();
};

