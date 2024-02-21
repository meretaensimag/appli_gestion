#include "JsonParser.hpp"
#include <iostream>
#include <fstream>


JsonParser::JsonParser(const std::string& filePath) {
    std::ifstream file(filePath);
    if (!file.is_open()) {
        throw std::runtime_error("Failed to open the JSON file.");
    }
    file >> j;
}

double JsonParser::parseDom(std::vector<Currency> currencies){
    return currencies[0].domesticInterestRate_;
}

std::pair<std::vector<Currency>, std::vector<double>> JsonParser::parseCurrencies() {
    PnlMat *correlation;
    j.at("Correlations").get_to(correlation);
    //pnl_mat_print(correlation);

    int size = correlation->n;
    //pnl_mat_free(&correlation); 

    int AssetsNbr = (int)j["Assets"].size();


    //Remplissage de Currencies
    std::vector<Currency> currencies;
    std::vector<double> foreignInterestRates;


    pnl_mat_chol(correlation);
    auto begin= j.at("Currencies").begin();

    double dom = (*begin)["InterestRate"];
    int compteur = AssetsNbr ;
    for (auto item: j.at("Currencies")){
        if(item["id"] == j.at("DomesticCurrencyId")){
            double interestRate = item["InterestRate"];
            double volatility = item["Volatility"];
        }
        else{
            double interestRate = item["InterestRate"];
            double volatility = item["Volatility"];
           
            PnlVect* Lrow= pnl_vect_create_from_zero(size);
            pnl_mat_get_row(Lrow, correlation, compteur);
    
            pnl_vect_mult_scalar(Lrow, volatility);
            
            double drift = dom;
            Currency* myCurrency = new Currency(drift,
                                           Lrow,
                                           interestRate,
                                           dom);
            currencies.push_back(*myCurrency);
            foreignInterestRates.push_back(myCurrency->interestRate_);
            compteur ++;
        }

    }
    return std::make_pair(currencies, foreignInterestRates);
}

std::pair<std::vector<Asset>, std::vector<int>> JsonParser::parseAssets(const std::vector<Currency>& currencies) {
    std::vector<Asset> assets;
    std::vector<int> assetCurrencyMapping;
    //A factoriser ca apres et le mettre comme attributs
    PnlMat *cor;
    j.at("Correlations").get_to(cor);
    //pnl_mat_print(correlation);
    pnl_mat_chol(cor);


    auto begin= j.at("Currencies").begin();

    double dom = (*begin)["InterestRate"];

    int size = cor->n;
    if (!j.at("Assets").empty()) {
        int n = 0;
        std::string currency_id;
        std::string ancien_id = j.at("DomesticCurrencyId");
        PnlVect* volX = pnl_vect_create_from_zero(size);
        auto c = -1;
        for (auto &item: j.at("Assets")) {
            currency_id = item["CurrencyId"];
            if (currency_id == j.at("DomesticCurrencyId")) {
                double volatility = item["Volatility"];
                PnlVect *Lrow = pnl_vect_create_from_zero(size);
                pnl_mat_get_row(Lrow, cor, n);
                pnl_vect_mult_scalar(Lrow, volatility);
                n++;
                Asset* myDomesticAsset = new Asset(dom, dom, Lrow);
                assets.push_back(*myDomesticAsset);
                assetCurrencyMapping.push_back(0);
            }
            else{
                double volS = item["Volatility"];
                PnlVect *LrowS = pnl_vect_create_from_zero(size);
                pnl_mat_get_row(LrowS, cor, n);
                pnl_vect_mult_scalar(LrowS, volS);
                if(ancien_id != currency_id){
                    c++;
                    
                    pnl_vect_plus_vect(LrowS, currencies[c].volatilityVector_);
                }
                else{
                    pnl_vect_plus_vect(LrowS, currencies[c].volatilityVector_);
                }

                n++;
                Asset* myAsset = new Asset(dom, dom, LrowS );
                assets.push_back(*myAsset);
                assetCurrencyMapping.push_back(c+1);
                ancien_id = currency_id;
            }
        }
    //pnl_mat_free(&cor);
    }
    return std::make_pair(assets, assetCurrencyMapping);
}


std::string JsonParser::parseOptionType() {
    std::string option_type = j["Option"]["Type"].get<std::string>();
    return option_type;
}

double JsonParser::parseStrike(const std::string& optionType) {
    double strike = (optionType == "foreign_asian") ? -1.0 : j["Option"]["Strike"].get<double>();
    return strike;
}


PnlMat *JsonParser::parsePast(PnlMat* marketData, TimeGrid *timeGrid, const std::vector<int>& assetCurrencyMapping, const std::vector<double>& foreignInterestRates){
    int RowCount = marketData->m;
    //PnlMat* past = pnl_mat_create(marketData->m, columnCount);
    PnlVect* dataLine= pnl_vect_create(RowCount);
    int n_bar = foreignInterestRates.size();
    for(int i=0; i<RowCount; i++) {
        pnl_mat_get_row(dataLine, marketData, i);
        int n0 = (int) std::count(assetCurrencyMapping.begin(), assetCurrencyMapping.end(), 0);
        for (int l = 0; l < n0; l++) {
            pnl_mat_set(marketData, i, l, GET(dataLine, l));
        }
        for (int idx = 0; idx < n_bar; idx++) {
            int n_idx = (int) std::count(assetCurrencyMapping.begin(), assetCurrencyMapping.end(), idx + 1);
            for (int k = 0; k < n_idx; k++) {
                pnl_mat_set(marketData, i, k + n0, GET(dataLine, k + n0) * GET(dataLine, idx + assetCurrencyMapping.size()));
            }
            n0 += n_idx;
        }
        for (int h = 0; h < n_bar; h++) {
            //std::cout << "L interest raate houa hada" << std::endl;
            //std::cout << foreignInterestRates[h] << std::endl;
            pnl_mat_set(marketData, i, h + (int) assetCurrencyMapping.size(),
                        GET(dataLine, h + assetCurrencyMapping.size()) * exp(foreignInterestRates[h] * (i / (double)252)));
        }
    }
    return marketData;
}