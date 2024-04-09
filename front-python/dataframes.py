from os import path
import os
import numpy as np
import pandas as pd
from constantes import *
from math import isinf
from sys import stderr
from datetime import datetime, timedelta

# Ou définissez le chemin de manière explicite
data_path = '../data'


# DATA FROM INFO FILE
nb_assets = 5
nb_foreign_currencies = 3
nb_considered_underlyings = 8

assets_order = [EUROSTOXX50,	MIB, FTSE100,	NIKKEI,	SENSEX]
currency_interest_rates_names = [REUR, RGBP, RJPY, RINR ]
change_currency_names = [XGBP, XJPY, XINR]

assets_currency_order = assets_order + currency_interest_rates_names
assets_currency_except_reur = [EUROSTOXX50,	MIB, FTSE100,	NIKKEI,	SENSEX, RGBP, RJPY, RINR ]

asset_to_currency = {
    EUROSTOXX50: None,
    FTSE100:  XGBP,
    MIB: None,
    NIKKEI: XJPY,
    SENSEX: XINR,
    RGBP:    XGBP,
    RJPY:    XJPY,
    RINR:    XINR,
    REUR: None
}

underlyings = assets_order + change_currency_names

_dataframe_dtypes = dict(zip(assets_order, (np.float64 for _ in range(nb_assets))))  # Rajout type asset

_dataframe_dtypes.update(
    dict(zip(change_currency_names, (np.float64 for _ in range(nb_foreign_currencies))))  # Rajout type des taux changes
)

_dataframe_dtypes.update(
    dict(zip(currency_interest_rates_names, (np.float64 for _ in range(nb_foreign_currencies+1))))  # Rajout interests
)

_dataframe_dtypes["Date"] = str  # Rajout date



# DATAFRAMES BUILDING

_close_price = path.join(data_path, "ClosePrice.xlsx")
close_price = pd.read_excel(_close_price, dtype=_dataframe_dtypes, na_values="Nan",
                          usecols=["Date"] + assets_order, index_col=0)

# Convert the index to datetime if it's not already
close_price.index = pd.to_datetime(close_price.index)

# Then format the index to the desired string format
close_price.index = close_price.index.strftime('%d-%m-%Y')

_close_ret = path.join(data_path, "CloseRet.xlsx")
close_ret = pd.read_excel(_close_ret, dtype=_dataframe_dtypes, na_values="Nan",
                        usecols=["Date"] + assets_order, index_col=0)

# Convert the index to datetime if it's not already
close_ret.index = pd.to_datetime(close_ret.index)

# Then format the index to the desired string format
close_ret.index = close_ret.index.strftime('%d-%m-%Y')

_taux_interet = path.join(data_path, "ti.xlsx")
taux_interet = pd.read_excel(_taux_interet, dtype=_dataframe_dtypes, na_values="Nan",
                           usecols=["Date"] + currency_interest_rates_names, index_col=0)

# Convert the index to datetime if it's not already
taux_interet.index = pd.to_datetime(taux_interet.index)

# Then format the index to the desired string format
taux_interet.index = taux_interet.index.strftime('%d-%m-%Y')

_xfor_price = path.join(data_path, "xforprice.xlsx")
xfor_price = pd.read_excel(_xfor_price, dtype=_dataframe_dtypes, na_values="Nan",
                         usecols=["Date"] + change_currency_names, index_col=0)

# Convert the index to datetime if it's not already
xfor_price.index = pd.to_datetime(xfor_price.index)

# Then format the index to the desired string format
xfor_price.index = xfor_price.index.strftime('%d-%m-%Y')

_xfor_ret = path.join(data_path, "xforret.xlsx")
xfor_ret = pd.read_excel(_xfor_ret, dtype=_dataframe_dtypes, na_values="Nan",
                       usecols=["Date"] + change_currency_names, index_col=0)

# Convert the index to datetime if it's not already
xfor_ret.index = pd.to_datetime(xfor_ret.index)

# Then format the index to the desired string format
xfor_ret.index = xfor_ret.index.strftime('%d-%m-%Y')

close_price = close_price.ffill()
close_ret = close_ret.ffill()
taux_interet = taux_interet.ffill()
xfor_price = xfor_price.ffill()
xfor_ret = xfor_ret.ffill()


# MERGED DATAFRAMES
daily_spots = pd.concat([close_price, xfor_price], axis=1)

desired_order = ['EUROSTOXX50', 'MIB', 'FTSE100', 'NIKKEI', 'SENSEX', 'XGBP', 'XJPY', 'XINR']

# Réordonner les colonnes du DataFrame 'df'
reordered_df = daily_spots[desired_order]

arithmetic_daily_rentas = pd.concat([close_ret, xfor_ret], axis=1)
log_daily_rentas = np.log(arithmetic_daily_rentas + 1)


# DATES and MAPPERS
nb_daily_dates = len(daily_spots.index)
_daily_dates_mapper = dict(zip(daily_spots.index, range(nb_daily_dates)))
_reverse_dates_mapper = list(daily_spots.index)



def daily_dates_mapper(string_date : str) -> int:
    """
    Fonction de mapping des dates
    :return: la int date correspondante
    """
    int_date = _daily_dates_mapper.get(string_date)
    if int_date is None:
        real_date = datetime.strptime(string_date, '%d-%m-%Y').date()
        one_day = timedelta(days=1)
        while int_date is None:
            #print("HELLO")
            real_date -= one_day
            int_date = _daily_dates_mapper.get(real_date.strftime('%d-%m-%Y'))
    return int_date


def reverse_dates_mapper(int_date):
    return _reverse_dates_mapper[int_date]



def time_from_january_the_first(int_date : int):
    """
    Retourne le nombre de jour entre int_date et le 1er janvier de la meme année.
    """
    spot_date = _reverse_dates_mapper[int_date]
    t = datetime.strptime(spot_date, '%d-%m-%Y')
    return t.timetuple().tm_yday


week_duration = 5


# GENERIC PARAMETERS
_third_option_dates = ("05-01-2009", "04-01-2010", "04-01-2011", "04-01-2012", "04-01-2013", "06-01-2014")

_second_option_dates = ("04-01-2005", "04-01-2006", "04-01-2007", "04-01-2008", "05-01-2009", "04-01-2010")

_first_option_dates = ("05-07-2000", "03-07-2001", "02-07-2002", "02-07-2003", "02-07-2004", "05-07-2005")


def get_saved_option_dates(option_number):
    """
    Retourne une copie des fixing dates string selon l'option number.
    """
    dates = None
    if option_number == 3:
        dates = _third_option_dates
    elif option_number == 2:
        dates = _second_option_dates
    elif option_number == 1:
        dates = _first_option_dates
    else:
        raise NotImplementedError(f"Option number {option_number} Not found in database")
    return list(dates)


_default_option_description = {
    Type: "Chorelia",
    MaturityInDays: None,
    FixingDatesInDays: {
        Type: "Grid",
       DatesInDays: None
    },
    ReferentialAmount : DefaultReferentialAmount,
}


def _load_assets():
    results = []
    for asset, currId in zip(assets_order, ["EUR", "EUR", "GBP", "JPY", "INR"]):
        d = {
            Id: asset,
            CurrencyId: currId
        }
        results.append(d)

    return results

def _load_currencies():
    results = []

    for currId in ["EUR", "GBP", "JPY", "INR"]:
        d = {
            id: currId,
            InterestRate: None
        }
        results.append(d)

    return results


_assets = _load_assets()
_currencies = _load_currencies()

_generic_test_parameters = {
    IsRebalancing: False,
    MathDate: None,
    DomesticCurrencyId:"EUR",
    Currencies: _currencies,
    Assets: _assets,
    NumberOfDaysInOneYear: DefaultNumberOfDaysInOneYear,
    Option: None,
    CovarianceMatrix: None,
    SampleNb: DefaultNbSample,
    RelativeFiniteDifferenceStep: DefaultFdSteps,
    Past: None,
}


def get_default_option_description():
    """
    :return: A template of the description of an option
    """
    return dict(_default_option_description)


def get_test_parameters():
    """
    :return: a template of the description of a test_parameter (pricer input)
    """
    parameters = dict(_generic_test_parameters)
    return parameters


def get_preload_option_description(option_number):
    parameters = dict(_default_option_description)
    dates = get_saved_option_dates(option_number)
    parameters[MaturityInDays] = dates[-1]
    parameters[FixingDatesInDays][DatesInDays] = dates
    return parameters


print("OK in DataFrames")

