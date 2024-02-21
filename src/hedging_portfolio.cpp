#include "JsonParser.hpp"
#include "MonteCarlo.hpp"
#include "Utils.hpp"
#include <iostream>
#include "fstream"
#include "FixedTimesOracle.hpp"
#include "Portfolio.hpp"

PnlMat* fillPast(PnlMat* hedgingPast, PnlVect* newSpot, int time, TimeGrid* timeGrid)
        {
            PnlMat* past = pnl_mat_new();

            for (int i = 0; i < hedgingPast->m; ++i) {
                pnl_mat_resize(past, past->m+1, newSpot->size);
                PnlVect* row = pnl_vect_create(newSpot->size);
                pnl_mat_get_row(row, hedgingPast, i);
                pnl_mat_set_row(past, row, i);
                pnl_vect_free(&row);

            }
            pnl_mat_resize(past, past->m+1, newSpot->size);
            pnl_mat_set_row(past, newSpot, past->m-1);


            if (timeGrid->isMonitoringDate(time))
            {
                pnl_mat_resize(hedgingPast, hedgingPast->m+1, newSpot->size);
                pnl_mat_set_row(hedgingPast, newSpot, hedgingPast->m-1);
                
            }
            return past;
}


int main(int argc, char *argv[]) {

        JsonParser parser(argv[1]);
        std::vector<Currency> currencies;
        std::vector<double> foreignInterestRates;
        std::tie(currencies, foreignInterestRates) = parser.parseCurrencies();
        std::vector<Asset> assets;
        std::vector<int> assetCurrencyMapping;
        std::tie(assets, assetCurrencyMapping) = parser.parseAssets(currencies);


        Utils* outil = nullptr;
       
        std::string optionType = parser.parseOptionType();
       
        double strike_ = parser.parseStrike(optionType);
       
        double dom = parser.parseDom(currencies);
       
        auto* timegrid = new TimeGrid(parser.j);

        Option* opt = outil->createOption(optionType, assetCurrencyMapping,foreignInterestRates,timegrid, dom, strike_);

        double fdStep_;
        int nbSamples_;
        parser.j.at("RelativeFiniteDifferenceStep").get_to(fdStep_);
        parser.j.at("SampleNb").get_to(nbSamples_);
        
        PnlMat* hedgingPast = pnl_mat_new();
        GlobalModel *mod = new GlobalModel(assets, currencies, timegrid, hedgingPast);
        // Instanciation de notre classe Monte Carlo
        MonteCarlo* mc = new MonteCarlo(opt, mod, fdStep_, nbSamples_);

        // On fait respectivement appel aux méthodes price et resimulation de Monte Carlo

        Portfolio hedgingPortfolio = Portfolio(*mc);


        int period; 

        parser.j.at("PortfolioRebalancingOracleDescription").at("Period").get_to(period);

        FixedTimesOracle* fixedTimesOracle = new FixedTimesOracle(timegrid, period);

        PnlMat* marketData = pnl_mat_create_from_file(argv[2]);
        marketData = parser.parsePast(marketData, timegrid, assetCurrencyMapping, foreignInterestRates);

        double price = 0.0;
        double priceStdDev = 0.0;
        int size = marketData->n;
        double riskFreeRate = hedgingPortfolio.monteCarlo.option_->domesticInterestRate_; 
        PnlVect* initDeltas = pnl_vect_create(size);
        PnlVect* deltasStdDev = pnl_vect_create(size);
        double initialPrice = 0.0;
        double initPriceStdDev = 0.0; 
        PnlVect* initSpots = pnl_vect_create(size);
        pnl_mat_get_row(initSpots, marketData, 0);

        PnlMat* initPastData = fillPast(hedgingPast, initSpots, 0, timegrid); 
        hedgingPortfolio.monteCarlo.priceAndDelta(initPastData, 0, initialPrice, initPriceStdDev, initDeltas, deltasStdDev);
        double portfolio = initialPrice;

        Position position = Position(0, initialPrice, initPriceStdDev, initDeltas, deltasStdDev, portfolio);
        hedgingPortfolio.positions.push_back(position);
        double CashValue = initialPrice - pnl_vect_scalar_prod(initDeltas, initSpots);
        double currentCashValue = 0.0;

        PnlVect* deltas = pnl_vect_create(size);
        int oldDate = 0;
    
        for (int i = 1; i < marketData->m-1; i++) {
            PnlVect* vectPrices = pnl_vect_create(size);
            pnl_mat_get_row(vectPrices, marketData, i);
            PnlMat* pastData = fillPast(hedgingPast, vectPrices, i, timegrid);
            deltas = pnl_vect_create_from_scalar(size, 0.0);
            hedgingPortfolio.monteCarlo.priceAndDelta(pastData, i, price, priceStdDev, deltas, deltasStdDev);
            int capTime = i - oldDate;
            currentCashValue = CashValue*exp(riskFreeRate * capTime/(double)252);
            portfolio = currentCashValue + pnl_vect_scalar_prod(initDeltas, vectPrices);
            oldDate = i; 
        if (fixedTimesOracle->rebalancingDate(i)){
            currentCashValue = portfolio - pnl_vect_scalar_prod(deltas, vectPrices);
            Position position = Position(i, price, priceStdDev, deltas, deltasStdDev, portfolio);
            hedgingPortfolio.positions.push_back(position);
        }
        pnl_vect_free(&vectPrices);
        CashValue = currentCashValue;
        initDeltas = deltas;
        price = 0.0;
    }

        nlohmann::json jsonPortfolio = hedgingPortfolio.positions;
        std::ofstream ifout(argv[3], std::ios_base::out);
        if (!ifout.is_open()) {
        std::cout << "Unable to open file " << argv[3] << std::endl;
        std::exit(1);
        }
        ifout << jsonPortfolio.dump(4);
        ifout.close();

        pnl_vect_free(&initDeltas);
        pnl_vect_free(&deltasStdDev);
        pnl_vect_free(&initSpots);
        pnl_mat_free(&initPastData);
        pnl_mat_free(&hedgingPast);
        pnl_mat_free(&marketData);
    return 0;
}
