from dash import dcc, html, Input, Output
from datetime import datetime
import app

def gestion_portefeuille_layout():
    return html.Div(children=[
        html.Div(children=[
            html.H1(children='Gestion de Portefeuille', className='mt-3'),
            html.Div(children='''
                Bienvenue sur la page de gestion de portefeuille !
            ''', className='description'),

            html.Div(children=[
                dcc.DatePickerSingle(
                    id='date-picker',
                    date=datetime.now(),
                    display_format='DD-MM-YYYY',
                ),
                html.Button('Prévisualisation', id='preview-button', n_clicks=0),
                html.Button('Rééquilibrage', id='rebalance-button', n_clicks=0),
            ], className='mt-3'),

        ], className='content')
    ])


def update_date_and_button_clicks(selected_date, preview_clicks, rebalance_clicks):
    # Ici, selected_date contient la valeur de la date sélectionnée dans le DatePickerSingle
    # Vous pouvez l'utiliser comme vous le souhaitez dans votre application
    return selected_date, preview_clicks, rebalance_clicks