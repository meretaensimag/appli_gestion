#include "parsing.hpp"

double Parser::getDomesticRate(){
    return domesticInterestRate;
};


Option* Parser::getOption(){
    return internalOption;
};


std::vector<RiskyAsset*>* Parser::getAssets(){
    return &internalRiskyAssets;
};


std::vector<int>* Parser::getAssetsMapping(){
    return &mapping;
};


int Parser::getNbSamples(){
    return internalJSON["Option"].at("Sample").get<double>();
};


double Parser::getFdStep(){
    return internalJSON["Option"].at("RelativeFiniteDifferenceStep").get<double>();
};


bool Parser::getIsFixingDate(){
    for (int i=0; i<internalOption->dates_->size; i++){
        if (GET(internalOption->dates_, i) == spot_date)
            return true;
    }
    return false;
};


double Parser::getTime(){
    double result = spot_date;
    return result / nb_days_in_year;
};


bool Parser::getIsMaturityDate(){
    return spot_date == GET(internalOption->dates_, internalOption->dates_->size-1);
};


PnlMat* Parser::getPast(){
    return Past;
};

int Parser::getSize(){
    return internalRiskyAssets.size();
};


void Parser::build_correlation_matrix(){
    // Recuperation volatilités
    PnlVect* vol = pnl_vect_create(Correlation->m);
    for (int i=0; i < Correlation->m; i++)
        LET(vol, i) = sqrt(MGET(Correlation, i, i));

    // Passage des covariances aux correlations
    for (int i=0; i < Correlation->m; i++){
        for (int j=0; j < Correlation->m; j++){
            MLET(Correlation, i, j) /= (GET(vol, i) * GET(vol, j));
        }
    }

    // Application de cholesky
    pnl_mat_chol(Correlation);

    // On remultiplie chaque ligne par sa volatilite
    for (int i=0; i < Correlation->m; i++)
        for (int j=0; j < Correlation->m; j++)
            MLET(Correlation, i, j) *= GET(vol, i);

    return;
}

void Parser::build_assets_and_mapping_vector(){
    int risky_asset_index = 0;
    for (auto risky_asset_json: internalJSON["UnderlyingAssets"]){
        RiskyAsset * riskyAsset;
        PnlVect vol = pnl_vect_wrap_mat_row(Correlation, risky_asset_index);
        std::string type = risky_asset_json.at("Type").get<std::string>();
        std::string id = risky_asset_json.at("Id").get<std::string>();
        if (type == "Asset"){
            int currency_index = 0;
            bool is_local_asset = true;
            std::string currency_id = risky_asset_json.at("CurrencyId").get<std::string>();
            double currency_interest_rate;
            for(auto currency_json: internalJSON["UnderlyingAssets"]){
                std::string type = currency_json.at("Type").get<std::string>();
                if(type == "Currency" && currency_id == currency_json.at("Id").get<std::string>()){
                    currency_interest_rate = currency_json.at("InterestRate").get<double>();
                    is_local_asset = false;
                    break;
                }
                currency_index += 1;
            }
            double drift;
            if (is_local_asset){
                drift = domesticInterestRate;
                currency_index = -1;
            } else {
                PnlVect sigma_x = pnl_vect_wrap_mat_row(Correlation, currency_index);
                drift = currency_interest_rate - pnl_vect_scalar_prod(&vol, &sigma_x);
            }
            mapping.push_back(currency_index);
            riskyAsset =  new Asset(drift, &vol);

        } else if (type == "Currency"){
            int asset_index = 0;
            for(auto asset_json : internalJSON["UnderlyingAssets"]){
                std::string type = asset_json.at("Type").get<std::string>();
                if(type == "Asset" && id == asset_json.at("CurrencyId").get<std::string>()){
                    break;
                }
                asset_index += 1;
            }
            mapping.push_back(asset_index);
            riskyAsset = new Currency(domesticInterestRate, risky_asset_json.at("InterestRate").get<double>(), &vol);
        } else {
            throw std::invalid_argument("Asset type not handled\n");
        }
        internalRiskyAssets.push_back(riskyAsset);
        risky_asset_index += 1;

    }
    return;
}


void Parser::build_option(){
    std::string optLabel = internalJSON.at("Option").at("Type").get<std::string>();
    if (optLabel != "yosemite"){
        throw std::invalid_argument("Option not handled by this pricer");
    }
    auto option = internalJSON["Option"];
    PnlVect* dates;
    option.at("FixingDates").at("Dates").get_to(dates);
    for (int i= 0; i < dates->size; i++)
        LET(dates, i) = GET(dates, i) / nb_days_in_year; 

    return;
}

Parser::Parser(nlohmann::json & js){

    internalJSON = js;
    spot_date = internalJSON.at("MathDate").get<int>();
    nb_days_in_year = internalJSON.at("NumberOfDaysInOneYear").get<int>();

    internalJSON.at("Past").get_to(Past);

    internalJSON.at("CovarianceMatrix").get_to(Correlation);
    build_correlation_matrix();

    domesticInterestRate = internalJSON["DomesticInterestRate"].at("InterestRate").get<double>();
    // La domestic interest rate est requise pour les deux fonctions suivantes

    build_option();

    build_assets_and_mapping_vector();
}

Parser::Parser(const std::string& inputString) {
    internalJSON = nlohmann::json::parse(inputString);
    spot_date = internalJSON.at("MathDate").get<int>();
    nb_days_in_year = internalJSON.at("NumberOfDaysInOneYear").get<int>();

    internalJSON.at("Past").get_to(Past);

    internalJSON.at("CovarianceMatrix").get_to(Correlation);
    build_correlation_matrix();

    domesticInterestRate = internalJSON["DomesticInterestRate"].at("InterestRate").get<double>();
    // La domestic interest rate est requise pour les deux fonctions suivantes

    build_option();

    build_assets_and_mapping_vector();
};


Parser::~Parser(){
    pnl_mat_free(&Correlation);
    pnl_mat_free(&Past);
};