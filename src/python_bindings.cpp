#include <pybind11/pybind11.h>
#include <pybind11/stl.h>
#include <pybind11/numpy.h>
#include "JsonParser.hpp"

#include "MonteCarlo.hpp"

namespace py = pybind11;

py::list pnl_to_py_list(const PnlVect *vect) {
    py::list res;
    for (int i = 0; i < vect->size; ++i) {
        res.append(GET(vect, i));
    }
    return res;
}

py::tuple price_and_delta(const std::string &inputString) {
    JsonParser parser(inputString);

    double fdStep_;
    int nbSamples_;
    parser.j.at("RelativeFiniteDifferenceStep").get_to(fdStep_);
    parser.j.at("SampleNb").get_to(nbSamples_);



    PnlMat *pastData;
    parser.j.at("Past").get_to(pastData);

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

    GlobalModel *mod = new GlobalModel(assets, currencies, timegrid, pastData);


    double computedPrice = 0.0;
    double priceVariance = 0.0;
    PnlVect *assetDeltas = pnl_vect_create_from_zero(pastData->n);
    PnlVect *deltaVariances = pnl_vect_create_from_zero(pastData->n);

    MonteCarlo monteCarlo(opt, mod, fdStep_, nbSamples_);
    monteCarlo.priceAndDelta(pastData, parser.getCurrentEvalDate(), computedPrice, priceVariance, assetDeltas, deltaVariances);

    py::list deltas = pnl_to_py_list(assetDeltas);
    py::list delta_variances = pnl_to_py_list(deltaVariances);

    pnl_vect_free(&assetDeltas);
    pnl_vect_free(&deltaVariances);
    pnl_mat_free(&pastData);

    return py::make_tuple(computedPrice, priceVariance, deltas, delta_variances);
}

PYBIND11_MODULE(pricer, m) {
    m.doc() = "Python wrapper for MonteCarlo price and delta calculation";

    m.def("price_and_delta", &price_and_delta, "Calculate price and delta");
}
