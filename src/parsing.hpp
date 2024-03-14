//
// Created by lionel on 15/03/23.
//

#pragma once
#include "nlohmann/json.hpp"
#include "json_reader.hpp"
#include "Asset.hpp"
#include "Currency.hpp"
#include "GlobalModel.hpp"
#include "MonteCarlo.hpp"
#include <stdexcept>
#include <cmath>


class Parser {
private:
    int spot_date;
    int nb_days_in_year;
    double domesticInterestRate;

    nlohmann::json internalJSON;
    std::vector<int> mapping;

    std::vector<RiskyAsset*> internalRiskyAssets;

    PnlMat* Past;
    PnlMat* Correlation;
    Option* internalOption;

    void build_correlation_matrix();
    void build_assets_and_mapping_vector();
    void build_option();

public:
    Parser(const std::string&  inputString);
    Parser(nlohmann::json & js);
    ~Parser();
    Option* getOption();
    double getDomesticRate();
    std::vector<RiskyAsset*>* getAssets();
    std::vector<int>* getAssetsMapping();
    int getNbSamples();
    double getFdStep();
    bool getIsFixingDate();
    bool getIsMaturityDate();
    double getTime();
    PnlMat* getPast();
    int getSize();

};