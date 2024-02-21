#pragma once
#include <vector>
#include "Asset.hpp"
#include "Currency.hpp"
#include "RiskyAsset.hpp"
#include "Option.hpp"
#include "pnl/pnl_random.h"
#include "json_reader.hpp"
#include "nlohmann/json.hpp"
#include "TimeGrid.hpp"

using namespace std;
using nlohmann::json;
class GlobalModel {
public:
    vector<Asset> assets_;
    vector<Currency> currencies_;
    vector<RiskyAsset> riskyAssets_;
    TimeGrid *timeGrid_;
    PnlVect* G_;
    PnlVect *vol_;
    PnlVect * spot_;
    PnlMat* hedgingPast; 

    GlobalModel(std::vector<Asset> assets, std::vector<Currency> currencies, TimeGrid* timeGrid, PnlMat* hedgingPast);
    ~GlobalModel();
    double discount();
    void sample(PnlMat* past, PnlMat* path, int t, PnlRng* rng);
    void stepSimulation(PnlVect *current_spot, double dt, int size, PnlVect * G);
    void shiftAsset(PnlMat *shift_path, const PnlMat *path, int d, double h, double t);
    int getIndex(std::vector<int> dates, int time);
    vector<RiskyAsset> createRiskyVector(vector<Currency> currencies, vector<Asset> assets);

};
