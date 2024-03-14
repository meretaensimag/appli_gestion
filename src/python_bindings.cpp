#include <pybind11/pybind11.h>
#include <pybind11/stl.h>
#include <pybind11/numpy.h>
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
    Parser parser(inputString);
    Option *opt = parser.getOption();
    GlobalModel *mod = parser.getGlobalModel();
    PnlMat *pastData = parser.getPastData();

    double computedPrice = 0.0;
    double priceVariance = 0.0;
    PnlVect *assetDeltas = pnl_vect_create_from_zero(mod->size_);
    PnlVect *deltaVariances = pnl_vect_create_from_zero(mod->size_);

    MonteCarlo monteCarlo(opt, mod, parser.getFdStep(), parser.getNbSamples());
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
