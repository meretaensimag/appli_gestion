#pragma once

#include "Option.hpp"
#include <string>

/*
    La classe Utils fonctionne comme une classe Factory pour la création d'objets Options de différents types destinés à être valorisés.
    Cette méthode crée une instance de l'option selon le type spécifié dans le fichier JSON et retourne un pointeur vers l'option créée.
*/
class Utils {
public:
    Option* createOption(const std::string& optionType, std::vector<int> assetCurrencyMapping,std::vector<double> foreignInterestRates,TimeGrid *timeGrid, double domesticInterestRate, double strike);

};