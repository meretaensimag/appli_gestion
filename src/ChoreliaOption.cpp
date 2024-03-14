#include "Option.hpp"


ChoreliaOption :: ChoreliaOption(std::vector<int> assetCurrencyMapping,
                                           std::vector<double> foreignInterestRates,
                                           TimeGrid *timeGrid,
                                           double domesticInterestRate,
                                           double referential_amount) {
    this->assetCurrencyMapping_ = std::move(assetCurrencyMapping);
    this->foreignInterestRates_ = std::move(foreignInterestRates);
    this->domesticInterestRate_ = domesticInterestRate;
    this->timeGrid_ = timeGrid;
    this->referential_amount_ = referential_amount;
}


double ChoreliaOption :: adjust_rate(double rate) {
    // La performance est limitee à la baisse à -15%
    if (rate < -0.15) {
        return -0.15;
    }
    return rate;
}

// Calcule la moyenne des taux de rendement après exclusion du meilleur et du pire
double ChoreliaOption :: calculate_average_rate(const PnlMat *path, int index) {
    std::vector<double> rates(5);
    // Remplit le vecteur avec les taux de rendement des indices
    for (int j = 0; j < 5; ++j) {
        double spot_at_0 = pnl_mat_get(path, 0, j);
        double spot_at_t = pnl_mat_get(path, index, j);
        double rate = (spot_at_t - spot_at_0) / spot_at_0;
        rates[j] = adjust_rate(rate);
    }

    // Trier le vecteur pour pouvoir exclure la meilleure et la pire rentabilite
    //On pose le plus petit dans rates[0]
    std::nth_element(rates.begin(), rates.begin() + 1, rates.end());
    //On pose le plus grand au dernier indice
    std::nth_element(rates.begin() + 1, rates.end() - 1, rates.end(), std::greater<>());
    // Calculer la moyenne en excluant le premier et le dernier élément
    double sum = std::accumulate(rates.begin() + 1, rates.end() - 1, 0.0);

    return sum / 3; // Retourner la moyenne des 3 rentabilites intermediaires
}

double ChoreliaOption :: payoff(const PnlMat *path) {
    double final_perf = 1.0; // Cumul de la performance annuelle
    double payoff = 0.;
    double dividend = 0.0;   // Dividende à payer

    for (int i = 1; i < path->m; ++i) {
        double annual_perf = calculate_average_rate(path, i);
        final_perf *= (1 + annual_perf);

        // Paiement de dividendes pour l'indice avec la 3ème meilleure performance
        if (i < path->m - 1) {
            std::vector<double> rates(5);
            for (int j = 0; j < 5; ++j) {
                double spot_at_0 = pnl_mat_get(path, 0, j);
                double spot_at_t = pnl_mat_get(path, i, j);
                rates[j] = (spot_at_t - spot_at_0) / spot_at_0;
            }
            std::nth_element(rates.begin(), rates.begin() + 2, rates.end(), std::greater<>());
            /*si l’indice du panier qui a la 3ème rentabilité annuelle la plus grande (R[3]),
             * a une rentabilité positive, on verse au porteur un dividende de
            25€ x R[3]*/
            if(rates[2] >0){
                dividend = 25 * rates[2]; // La troisième meilleure performance

            }
        }
            /*On verse au porteur à l‘échéance la valeur liquidative de référence augmentée (ou diminuée)
de 25% de cette performance finale (appliquée à la valeur liquidative de référence).*/
        else{
            dividend = referential_amount_ * (1 + 0.25*final_perf); // 25% de la performance finale
        }
        payoff += dividend * std::exp(domesticInterestRate_ *(timeGrid_->maturity_ -timeGrid_->dateList_[i]));
    }
    return payoff;
}
