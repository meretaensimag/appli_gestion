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

    //parser.j.at("PortfolioRebalancingOracleDescription").at("Period").get_to(period);

    //FixedTimesOracle* fixedTimesOracle = new FixedTimesOracle(timegrid, period);

    //PnlMat* marketData = pnl_mat_create_from_file(argv[2]);
    //marketData = parser.parsePast(marketData, timegrid, assetCurrencyMapping, foreignInterestRates);
    PnlMat* marketData;
    //pnl_mat_print(marketData);
    parser.j.at("Past").get_to(marketData);
    parser.parsePast(marketData, timegrid , assetCurrencyMapping, foreignInterestRates);

    //std::cout << "asset currreeeeeency mapping" << std::endl;
//    for(double el: foreignInterestRates){
//        std::cout << el << std::endl;
//    }

    //pnl_mat_print(marketData);
    double price = 0.0;
    double priceStdDev = 0.0;
    int size = marketData->n;
    double riskFreeRate = hedgingPortfolio.monteCarlo.option_->domesticInterestRate_;
    PnlVect* initDeltas = pnl_vect_create(size);
    PnlVect* deltasStdDev = pnl_vect_create(size);
    double initialPrice = 0.0;
    double initPriceStdDev = 0.0;
    PnlVect* initSpots = pnl_vect_create(size);
    //pnl_mat_get_row(initSpots, marketData, 0);
    //pnl_mat_print(marketData);
    //PnlMat* initPastData = fillPast(hedgingPast, initSpots, 0, timegrid);
    hedgingPortfolio.monteCarlo.priceAndDelta(marketData, parser.j.at("MathDate").get<int>(), initialPrice, initPriceStdDev, initDeltas, deltasStdDev);
    double portfolio = initialPrice;

    Position position = Position(0, initialPrice, initPriceStdDev, initDeltas, deltasStdDev, portfolio);
    hedgingPortfolio.positions.push_back(position);

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
    //pnl_mat_free(&initPastData);
    pnl_mat_free(&hedgingPast);
    pnl_mat_free(&marketData);
//
////    PnlVect* constaDates = pnl_vect_create_from_zero(4);
////    pnl_vect_set(constaDates, 0, 260);
////    pnl_vect_set(constaDates, 1, 521);
////    pnl_vect_set(constaDates, 2, 782);
////    pnl_vect_set(constaDates, 3, 1044);
////
////    PnlVect* interestRates = pnl_vect_create_from_scalar(2, 0.2);
////
////    //eurostralProduct product = eurostralProduct(50, 0.1, 252, constaDates, interestRates);
////
//////    Scenario 1
//PnlMat* path = pnl_mat_create_from_list(6, 8,
//                                        2553.41, 20262.0, 4579.64, 9043.12, 10275.6, 1.08041752425757, 0.0078727419755739, 0.0151875602946144,
//                                        3017.8, 23545.02, 5500.34, 10654.79, 17558.73, 1.11685518007078, 0.00749772415955292, 0.0149970687479279,
//                                        2844.17, 20547.03, 6013.87, 10398.1, 20498.72, 1.17164122998271, 0.00916522212836933, 0.0167443913405376,
//                                        2349.89, 15327.03, 5668.45, 8560.11, 15882.64, 1.20672333848532, 0.01007952950369, 0.0146224697643881,
//                                        2709.35, 16959.78, 6089.84, 10688.11, 19784.08, 1.22989209458942, 0.00868459600353651, 0.0139320334941132,
//                                        3069.16, 19233.74, 6730.73, 15908.88, 20787.3, 1.20378660013209, 0.0070419080213156, 0.0117771984416835
//);
//PnlMat* path = pnl_mat_create_from_list(6, 8,
//    5166.25, 46606.94, 6382.46, 17435.95, 4912.11, 1.58201280302235, 0.00981507726998101, 0.0235297771683043,
//    4243.8, 37841.91, 5639.91, 12817.41, 3312.29, 1.65861134032771, 0.00947763502753452, 0.0250441522142499,
//    2999.17, 27405.37, 4546.77, 10622.32, 3285.04, 1.55730223123732, 0.00846435305426023, 0.02079554609312,
//    2420.35, 24403.98, 4006.86, 9592.24, 3601.39, 1.44408807212205, 0.0073406011940633, 0.018718453789828,
//    2783.99, 27909.0, 4407.4, 11721.49, 4870.58, 1.48575602629657, 0.00749697425867407, 0.0177792427873835,
//    3207.91, 32553.0, 5190.14, 11616.7, 7220.25, 1.47456346541303, 0.00751691968417872, 0.0192986112719329);
//    parser.parsePast(path, timegrid , assetCurrencyMapping, foreignInterestRates);
//
//
//////    Scenario 2
////// PnlMat* path = pnl_mat_create_from_list(6, 8,
////
//////    24848.04, 4290.5, 9142.98,
//////    35002.37, 5523.62, 11501.48,
//////    44019.77, 6674.4, 12553.6,
//////
//////    61036.61, 7808.69, 13778.58,
//////
//////    41518.66, 4983.99,  9285.51,
//////    70045.08, 6048.3,  11866.9);
////
////
//////    Scenario 3
//////    PnlMat* path = pnl_mat_create_from_list(6, 3,
//////    41518.66, 4983.99, 9285.51,
//////    70045.08, 6048.3, 11866.9,
//////    70317.79, 6975.35, 13402.31,
//////    59364.95, 6111.55, 12226.47,
//////    62523.06, 7776.37, 12540.81,
//////    50973.62, 9428.0, 13495.54);
////    cout << "Path" <<  endl;
////    pnl_mat_print(path);
//    cout << "Paaaaaaaaaaaaaaaaaaaaaaaaaaaaaaath" <<  endl;
//    double dividend1 = opt->payoff(260, path);
//    double dividend2 = opt->payoff(521, path);
//    double dividend3 = opt->payoff(782, path);
//double dividend1 = opt->payoff(parser.j.at("MathDate").get<int>(), path);


//cout <<  endl;
//cout << "Dividend: " << endl;
//cout << dividend1 << endl;
//    cout << dividend2 << endl;
//    cout << dividend3 << endl;
//    cout << dividend4 << endl;
    cout <<  endl;

    return 0;
}
