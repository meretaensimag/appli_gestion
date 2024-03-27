import pandas as pd
import json
import os
from dataframes import *
from datetime import datetime
import subprocess
import numpy as np

def create_past_dataframe(int_date, parametre_tester,option_number):
    past_data = parametre_tester[Past]
    past_df = pd.DataFrame(past_data, columns=[f'Asset_{i}' for i in range(len(past_data[0]))])
    #on creer une colonne date avec les dates de la premiere option
    past_df['Date'] = parametre_tester[Option][FixingDatesInDays][DatesInDays][0:len(past_data)]
    return past_df

def update_interest_rates(new_interest_rates, parametre_tester):
    parametre_tester[Currencies][0][InterestRate] = new_interest_rates[REUR]
    parametre_tester[Currencies][1][InterestRate] = new_interest_rates[RGBP]
    parametre_tester[Currencies][2][InterestRate] = new_interest_rates[RJPY]
    parametre_tester[Currencies][3][InterestRate] = new_interest_rates[RINR]

def update_covariance_matrix(int_date, parametre_tester, nb_days=520):
    covar_math = log_daily_rentas.iloc[max(0, int_date - nb_days):int_date, :]
    parametre_tester[CovarianceMatrix] = covar_math.cov().to_numpy().tolist()

def spots_builder_for_past(int_date):
    spots = []
    for col in range(nb_considered_underlyings):
        spot = reordered_df.iat[int_date, col]
        past_date = int_date
        while np.isnan(spot) and past_date:
            past_date -= 1
            spot = reordered_df.iat[past_date, col]
        spots.append(spot)
    return spots

def update_past(int_date, parametre_tester):
    past = []
    for const_date in parametre_tester[Option][FixingDatesInDays][DatesInDays]:
        if const_date < int_date:
            past.append(spots_builder_for_past(const_date))
    past.append(spots_builder_for_past(int_date))
    parametre_tester[Past] = past

def remove_offset(parametre_tester):
    dates = parametre_tester[Option][FixingDatesInDays][DatesInDays]
    t_zero = dates[0]
    dates[:] = [date - t_zero for date in dates]
    parametre_tester[MathDate] -= t_zero
    parametre_tester[Option][MaturityInDays] -= t_zero

def verify_parametre_tester(parametre_tester):
    mat_date = parametre_tester[Option][MaturityInDays]
    dates = parametre_tester[Option][FixingDatesInDays][DatesInDays]
    if dates[-1] != mat_date:
        print(f"Last date doesn't match maturity. Last: {dates[-1]}, Maturity: {mat_date}", file=stderr)
        dates.append(mat_date)

def conv_dates(option):
    option[MaturityInDays] = daily_dates_mapper(option[MaturityInDays])
    const_dates = option[FixingDatesInDays][DatesInDays]
    option[FixingDatesInDays][DatesInDays] = [daily_dates_mapper(date) for date in const_dates]

def provide_parametre_tester(int_date, option, is_rebalancing):
    conv_dates(option)
    parametre_tester = get_test_parameters()
    parametre_tester[MathDate] = int_date
    parametre_tester[Option] = option
    parametre_tester[IsRebalancing] = is_rebalancing

    update_past(int_date, parametre_tester)
    update_interest_rates(taux_interet.iloc[int_date].to_dict(), parametre_tester)
    update_covariance_matrix(int_date, parametre_tester)
    remove_offset(parametre_tester)
    verify_parametre_tester(parametre_tester)
    return parametre_tester

def provide_parametre_tester_from_saved_option(int_date, option_num,is_rebalancing): 
    return provide_parametre_tester(int_date, get_preload_option_description(option_num),is_rebalancing)

def generate_output_json(filepath):
    # Chemin du répertoire actuel

    # Chemin du fichier de sortie JSON
    output_json_path = filepath

    # Date
    date = datetime(2001, 5, 3)
    date = daily_dates_mapper(date.strftime('%d-%m-%Y'))

    option_number = 1

    # boolean pour savoir si on rebalance ou non
    is_rebalancing = True
    # Création du paramètre testeur
    parametre_tester = provide_parametre_tester_from_saved_option(date, option_number, is_rebalancing)

    # Écriture du paramètre testeur dans le fichier JSON
    with open(output_json_path, 'w') as json_file:
        json.dump(parametre_tester, json_file, indent=4)


def get_one_spot_price(int_date, asset_code):
    """
    Given a spot_date and an asset_code, this function
    returns the spot prices of the given asset for the considered date
    """
    the_price = np.nan
    the_change_rate = 1
    if asset_code in underlyings:
        while np.isnan(the_price):
            all_prices = reordered_df.iloc[int_date]
            the_price = all_prices[asset_code]
            currency_code = asset_to_currency[asset_code]
            if currency_code is not None:
                the_change_rate = all_prices[currency_code]
                if np.isnan(the_change_rate):
                    the_price = np.nan
            int_date -= 1 # Retirer une date avant le prochain tour de boucle

    else:
        t = time_from_january_the_first(int_date)
        while np.isnan(the_price):
            all_prices = taux_interet.iloc[int_date]
            int_date -= 1
            the_price = np.exp(all_prices[asset_code] * t / 360)
            currency_code = asset_to_currency[asset_code]
            if currency_code is not None:
                the_change_rate = reordered_df.iloc[int_date][currency_code]
                if np.isnan(the_change_rate):
                    the_price = np.nan

    return {LOCAL_PRICE: the_price, EURO_PRICE: the_price * the_change_rate}


def get_portfolio_value(compos, int_date):
    """
    recupère la compo du ptf à la date int_date
    Calcule la valeur du ptf a cette date avec un produit scalaire.
    /!\ COMPO ORDONNEE
    """
    all_prices = [get_one_spot_price(int_date, asset_code)[EURO_PRICE] for asset_code in assets_currency_except_reur]

    return np.dot(all_prices, compos)


def get_modified_rate(x):
    if x < -0.15:
        return -0.15
    else:
        return x

    
def get_classic_dividend_rate(date, option_number):
    """
    Cette méthode prend en parametre une date de versement de divdendes et l'option_nb
    et retourne le taux du dividende qui doit etre versé à cette date
    """
    first_date = get_saved_option_dates(option_number)[0]
    first_int_date = daily_dates_mapper(first_date)
    current_int_date = daily_dates_mapper(date)

    spots_at_tO =  reordered_df.iloc[first_int_date]
    current_spots =  reordered_df.iloc[current_int_date]
    divid_rate = float('inf')
    for asset_code in assets_order:
        S0 = spots_at_tO[asset_code]
        S1 = current_spots[asset_code]
        renta = (S1 - S0 )/ S0
        #print( asset_code + " is " + str(renta))
        renta = get_modified_rate(renta)
        if renta < divid_rate and renta >= 0:
            divid_rate = renta
    if isinf(divid_rate):
        divid_rate = 0
    return divid_rate


def get_final_perf(option_number):
    """
    cette méthode sert à calculer la performance finale 
    de notre Produit Yosemite sur la periode spécifiée par option_number
    autrement pour une période, elle retourne le taux du dernier cashflow
    """
    final_perf = 0
    all_option_dates = get_saved_option_dates(option_number)
    first_int_date = daily_dates_mapper(all_option_dates[0])
    spots_at_t0 = reordered_df.iloc[first_int_date]

    for dividend_date in all_option_dates[1:]:
        current_int_date = daily_dates_mapper(dividend_date)
        current_spots = reordered_df.iloc[current_int_date]
        annual_perf = 0
        for asset_code in assets_order:
            S0 = spots_at_t0[asset_code]
            S1 = current_spots[asset_code]
            annual_perf += get_modified_rate((S1 - S0) / S0)
        annual_perf /= nb_assets
        final_perf += max(annual_perf, 0)
    return final_perf


def get_cash_flow_amount(date, option_number):
    """
    Calcul des dividendes aux dates de versement
    """
    if date == get_saved_option_dates(option_number)[-1]:
        return DefaultReferentialAmount * (1 + 0.25*get_final_perf(option_number))
    else:
        return DefaultReferentialAmount * get_classic_dividend_rate(date, option_number)


def is_dividend_date(date, option_number):
    option_dates = get_saved_option_dates(option_number)
    if date in option_dates:
        return True
    else:
        return False

def pay_dividend_and_rebalance(spot_date, option_number):
    rep = {}
    #ici on va lire le fichier sortie.json pour recuperer les anciennes compositions
    with open('sortie.json') as f:
        data = json.load(f)
    old_compos = data[-1]['deltas']
    int_date = daily_dates_mapper(spot_date)
    ptf_value = get_portfolio_value(old_compos, int_date)
    dividend_amount = get_cash_flow_amount(spot_date, int(option_number))
    pricing_params = provide_parametre_tester_from_saved_option(int_date, int(option_number),1)
    pricing_params = str(pricing_params).replace("'", '\"')
    command = ["./hedging_portfolio", "../../front-python/output.json", "../../front-python/sortie.json"]
    subprocess.run(command, cwd="../src/build")
    new_deltas = data[-1]['deltas']
    # on commence à remplir la réponse avec les 8 deltas dispos
    for i in range(len(assets_currency_except_reur)):
        rep[assets_currency_except_reur[i]] = new_deltas[i]
    #on calcule la somme à placer au taux sans risque euro bien attendu en retirant le dividende
    prices_except_reur = [get_one_spot_price(int_date, asset_code)[EURO_PRICE] for asset_code in assets_currency_except_reur]
    cash = ptf_value - dividend_amount
    cash -= np.dot(new_deltas, prices_except_reur)
    #on calcule la qte de zero coupons a acheter et on le set dans la rep
    delta_zc_euro = cash / get_one_spot_price(int_date, REUR)[EURO_PRICE]
    rep[REUR] = delta_zc_euro
    
    return rep

if __name__ == "__main__":
    # Exécuter la fonction pay_dividend_and_rebalance avec les arguments appropriés
    result = pay_dividend_and_rebalance("03-05-2001", 1)
    # Afficher le résultat
    print(result)
