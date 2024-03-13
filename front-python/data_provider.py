from dataframes import *
from datetime import datetime
import json
import os

def _update_interest_rates(new_interest_rates, parametre_tester):
    parametre_tester[Currencies][0][InterestRate] = new_interest_rates[REUR]
    parametre_tester[Currencies][1][InterestRate] = new_interest_rates[RGBP]
    parametre_tester[Currencies][2][InterestRate] = new_interest_rates[RJPY]
    parametre_tester[Currencies][3][InterestRate] = new_interest_rates[RINR]

def _update_covariance_matrix(int_date, parametre_tester, nb_days=520):
    covar_math = log_daily_rentas.iloc[max(0, int_date - nb_days):int_date, :]
    parametre_tester[CovarianceMatrix] = covar_math.cov().to_numpy().tolist()

def spots_builder_for_past(int_date):
    spots = []
    for col in range(nb_considered_underlyings):
        spot = daily_spots.iat[int_date, col]
        past_date = int_date
        while np.isnan(spot) and past_date:
            past_date -= 1
            spot = daily_spots.iat[past_date, col]
        spots.append(spot)
    return spots

def _update_past(int_date, parametre_tester):
    past = []
    for const_date in parametre_tester[Option][FixingDatesInDays][DatesInDays]:
        if const_date < int_date:
            past.append(spots_builder_for_past(const_date))
    past.append(spots_builder_for_past(int_date))
    parametre_tester[Past] = past

def _remove_offset(parametre_tester):
    dates = parametre_tester[Option][FixingDatesInDays][DatesInDays]
    t_zero = dates[0]
    dates[:] = [date - t_zero for date in dates]
    parametre_tester[MathDate] -= t_zero
    parametre_tester[Option][MaturityInDays] -= t_zero

def _verify_parametre_tester(parametre_tester):
    mat_date = parametre_tester[Option][MaturityInDays]
    dates = parametre_tester[Option][FixingDatesInDays][DatesInDays]
    if dates[-1] != mat_date:
        print(f"Last date doesn't match maturity. Last: {dates[-1]}, Maturity: {mat_date}", file=stderr)
        dates.append(mat_date)

def _conv_dates(option):
    option[MaturityInDays] = daily_dates_mapper(option[MaturityInDays])
    const_dates = option[FixingDatesInDays][DatesInDays]
    option[FixingDatesInDays][DatesInDays] = [daily_dates_mapper(date) for date in const_dates]

def provide_parametre_tester(int_date, option):
    _conv_dates(option)
    parametre_tester = get_test_parameters()
    parametre_tester[MathDate] = int_date
    parametre_tester[Option] = option

    _update_past(int_date, parametre_tester)
    _update_interest_rates(taux_interet.iloc[int_date].to_dict(), parametre_tester)
    _update_covariance_matrix(int_date, parametre_tester)
    _remove_offset(parametre_tester)
    _verify_parametre_tester(parametre_tester)
    return parametre_tester

def provide_parametre_tester_from_saved_option(int_date, option_num): 
    return provide_parametre_tester(int_date, get_preload_option_description(option_num))

current_dir = os.path.abspath(os.getcwd())

# Spécifier le nom du fichier de sortie
output_filename = '/user/4/.base/mereta/home/3A/appli_gestion/appli_gestion/front-python/output.json'
date = datetime(2004, 4, 1)
#on convertit la date en int
date = daily_dates_mapper(date.strftime('%d-%m-%Y'))

# Joindre le chemin absolu du répertoire actuel avec le nom du fichier de sortie
output_path = os.path.join(current_dir, output_filename)
with open(output_path, 'w') as f:
    json.dump(provide_parametre_tester_from_saved_option(date, 1), f, indent=4)
