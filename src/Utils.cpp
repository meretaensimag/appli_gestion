#include "Utils.hpp"
#include <iostream>
#include <string>
#include <vector>


/*
    La classe Utils fonctionne comme une classe Factory pour la création d'objets Options de différents types destinés à être valorisés.
    Cette méthode crée une instance de l'option selon le type spécifié dans le fichier JSON et retourne un pointeur vers l'option créée.
*/
Option* Utils::createOption(const std::string& optionType, std::vector<int> assetCurrencyMapping,std::vector<double> foreignInterestRates,TimeGrid *timeGrid, double domesticInterestRate, double strike) {
    Option* opt = nullptr;
    if (optionType == "call_quanto") {
        opt = new call_quanto(assetCurrencyMapping,foreignInterestRates, domesticInterestRate, strike);}
    else if (optionType == "call_currency") {
        opt = new call_currency(assetCurrencyMapping,foreignInterestRates, domesticInterestRate, strike);
    } else if (optionType == "quanto_exchange") {
        opt = new quanto_exchange(assetCurrencyMapping,foreignInterestRates, domesticInterestRate, strike);
    } else if (optionType == "foreign_asian") {
        opt = new foreign_asian(assetCurrencyMapping,foreignInterestRates, domesticInterestRate, strike);
    } else if (optionType == "foreign_perf_basket") {
        opt = new foreign_perf_basket(assetCurrencyMapping,foreignInterestRates,timeGrid, domesticInterestRate, strike);
    } else {
        // Gèstion du cas où le type d'option n'est pas reconnu
        std::cout << "unknown OptionType: " << optionType << std::endl;
    }
    return opt;
}