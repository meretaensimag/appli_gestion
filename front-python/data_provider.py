import pandas as pd
import json
import os
from dataframes import *
from datetime import datetime

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
        spot = daily_spots.iat[int_date, col]
        past_date = int_date
        while np.isnan(spot) and past_date:
            past_date -= 1
            spot = daily_spots.iat[past_date, col]
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

def provide_parametre_tester(int_date, option):
    conv_dates(option)
    parametre_tester = get_test_parameters()
    parametre_tester[MathDate] = int_date
    parametre_tester[Option] = option

    update_past(int_date, parametre_tester)
    update_interest_rates(taux_interet.iloc[int_date].to_dict(), parametre_tester)
    update_covariance_matrix(int_date, parametre_tester)
    remove_offset(parametre_tester)
    verify_parametre_tester(parametre_tester)
    return parametre_tester

def provide_parametre_tester_from_saved_option(int_date, option_num): 
    return provide_parametre_tester(int_date, get_preload_option_description(option_num))

# Chemin du répertoire actuel
current_dir = os.path.abspath(os.getcwd())

# Chemin du fichier de sortie JSON
output_json_path = os.path.join(current_dir, "output.json")

# Chemin du fichier de sortie CSV
output_csv_path = os.path.join(current_dir, "output.csv")

# Date
date = datetime(2004, 4, 1)
date = daily_dates_mapper(date.strftime('%d-%m-%Y'))

option_number = 1
# Création du paramètre testeur
parametre_tester = provide_parametre_tester_from_saved_option(date, option_number)

# Création du DataFrame de la matrice past
past_dataframe = create_past_dataframe(date, parametre_tester,option_number)

# Écriture du DataFrame dans le fichier CSV
past_dataframe.to_csv(output_csv_path, index=False)

# Écriture du paramètre testeur dans le fichier JSON
with open(output_json_path, 'w') as json_file:
    json.dump(parametre_tester, json_file, indent=4)
