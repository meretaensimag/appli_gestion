#include "JsonParser.hpp"
#include "MonteCarlo.hpp"
#include "Option.hpp"
#include <iostream>
#include "fstream"
//#include "FixedTimesOracle.hpp"
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


    double referentialAmount_ = parser.parseReferentialAmount();

    double dom = parser.parseDom(currencies);

    auto* timegrid = new TimeGrid(parser.j);
    Option* opt = new ChoreliaOption(assetCurrencyMapping,foreignInterestRates,timegrid, dom, referentialAmount_);

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

    
    PnlMat* marketData;
    parser.j.at("Past").get_to(marketData);
    parser.parsePast(marketData, timegrid , assetCurrencyMapping, foreignInterestRates);

    double price = 0.0;
    double priceStdDev = 0.0;
    int size = marketData->n;
    double riskFreeRate = hedgingPortfolio.monteCarlo.option_->domesticInterestRate_;
    PnlVect* initDeltas = pnl_vect_create(size);
    PnlVect* deltasStdDev = pnl_vect_create(size);
    double initialPrice = 0.0;
    double initPriceStdDev = 0.0;
    PnlVect* initSpots = pnl_vect_create(size);
    double MathDate =  parser.j.at("MathDate").get<int>();
    hedgingPortfolio.monteCarlo.priceAndDelta(marketData, MathDate, initialPrice, initPriceStdDev, initDeltas, deltasStdDev);
    double portfolio = initialPrice;

    Position position = Position(MathDate, initialPrice, initPriceStdDev, initDeltas, deltasStdDev, portfolio);
    
    hedgingPortfolio.positions.push_back(position);

    double Dom_riskfree  = initialPrice - pnl_vect_scalar_prod(initDeltas,initSpots);


    

    nlohmann::json jsonPortfolio = hedgingPortfolio.positions;
    std::ofstream ifout(argv[2], std::ios_base::out);
    if (!ifout.is_open()) {
        std::cout << "Unable to open file " << argv[2] << std::endl;
        std::exit(1);
    }
    ifout << jsonPortfolio.dump(4);
    ifout.close();
    std::cout << "Writting of new position done" << argv[2] << std::endl;
    pnl_vect_free(&initDeltas);
    pnl_vect_free(&deltasStdDev);
    pnl_vect_free(&initSpots);
    pnl_mat_free(&hedgingPast);
    pnl_mat_free(&marketData);
    return 0;
}

