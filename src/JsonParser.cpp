#include "JsonParser.hpp"
#include <iostream>
#include <fstream>


JsonParser::JsonParser(const std::string& filePath) {
    std::ifstream file(filePath);
    if (!file.is_open()) {
        throw std::runtime_error("Failed to open the JSON file.");
    }
    file >> j;
    j.at("CovarianceMatrix").get_to(Correlation);
    computeCorMatrix();
    pnl_mat_print(Correlation);

}

double JsonParser::parseDom(std::vector<Currency> currencies){
    return currencies[0].domesticInterestRate_;
}

void JsonParser::computeCorMatrix(){
    PnlVect* vol = pnl_vect_create(Correlation->m);
    for (int i=0; i < Correlation->m; i++){
        pnl_vect_set(vol, i,sqrt(pnl_mat_get(Correlation,i,i)));
    }
        

    for (int i=0; i < Correlation->m; i++){
        for (int k=0; k < Correlation->n; k++){

            pnl_mat_set(Correlation, i, k,pnl_mat_get(Correlation,i,k)/(pnl_vect_get(vol,i)*pnl_vect_get(vol,i)));
        }
    }
    pnl_vect_print(vol);
    pnl_mat_chol(Correlation);
    pnl_mat_print(Correlation);
    

    for (int l=0; l < Correlation->m; l++){
        for (int z=0; z < Correlation->n; z++){
            std::cout << "coucou" << std::endl;
            pnl_mat_set(Correlation, l, z,pnl_mat_get(Correlation,l,z)*pnl_vect_get(vol,l));
        }
    }    

}

std::pair<std::vector<Currency>, std::vector<double>> JsonParser::parseCurrencies() {

    int size = Correlation->n;
    int AssetsNbr = (int)j["Assets"].size();


    //Remplissage de Currencies
    std::vector<Currency> currencies;
    std::vector<double> foreignInterestRates;


    auto begin= j.at("Currencies").begin();

    double dom = (*begin)["InterestRate"];
    int compteur = AssetsNbr ;
    for (auto item: j.at("Currencies")){
        if(item["id"] == j.at("DomesticCurrencyId")){
            double interestRate = item["InterestRate"];
            //double volatility = item["Volatility"];
        }
        else{
            double interestRate = item["InterestRate"];
            //double volatility = item["Volatility"];
           
            PnlVect* Lrow= pnl_vect_create_from_zero(size);
            pnl_mat_get_row(Lrow, Correlation, compteur);
    
            //pnl_vect_mult_scalar(Lrow, volatility);
            
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


    auto begin= j.at("Currencies").begin();

    double dom = (*begin)["InterestRate"];

    int size = Correlation->n;
    if (!j.at("Assets").empty()) {
        int n = 0;
        std::string currency_id;
        std::string ancien_id = j.at("DomesticCurrencyId");
        PnlVect* volX = pnl_vect_create_from_zero(size);
        auto c = -1;
        for (auto &item: j.at("Assets")) {
            currency_id = item["CurrencyId"];
            if (currency_id == j.at("DomesticCurrencyId")) {
                //double volatility = item["Volatility"];
                PnlVect *Lrow = pnl_vect_create_from_zero(size);
                pnl_mat_get_row(Lrow, Correlation, n);
                //pnl_vect_mult_scalar(Lrow, volatility);
                n++;
                Asset* myDomesticAsset = new Asset(dom, dom, Lrow);
                assets.push_back(*myDomesticAsset);
                assetCurrencyMapping.push_back(0);
            }
            else{
                //double volS = item["Volatility"];
                PnlVect *LrowS = pnl_vect_create_from_zero(size);
                pnl_mat_get_row(LrowS, Correlation, n);
                //pnl_vect_mult_scalar(LrowS, volS);
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


double JsonParser::parseReferentialAmount(){
    return j["Option"]["ReferentialAmount"].get<double>();
}

double JsonParser::getCurrentEvalDate(){
    return ((double)j.at("MathDate").get<int>()) / 252;
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
            pnl_mat_set(marketData, i, h + (int) assetCurrencyMapping.size(),
                        GET(dataLine, h + assetCurrencyMapping.size()) * exp(foreignInterestRates[h] * (i / (double)252)));
        }
    }
    return marketData;
}

JsonParser::~JsonParser(){
    pnl_mat_free(&Correlation);
};