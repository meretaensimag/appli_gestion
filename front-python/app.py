import dash
from dash import dcc, html, Input, Output
import pandas as pd
import plotly.graph_objs as go
from datetime import datetime
import subprocess
import data_provider as dp
import dash_table
import json
import dataframes as dataf
from datetime import datetime

# Définir les feuilles de style externes
external_stylesheets = ['https://stackpath.bootstrapcdn.com/bootstrap/4.5.2/css/bootstrap.min.css', './assets/styles.css']


Listposition = []
OPTIONNUMBER = 1
DATE = datetime(2000,7,6)
# Créer l'application Dash      
app = dash.Dash(__name__, external_stylesheets=external_stylesheets)

# Charger les données depuis le fichier CSV
df = pd.read_excel('../data/ClosePrice.xlsx') 

# Convertir les colonnes de dates en objets datetime
df['Date'] = pd.to_datetime(df['Date'])

# Indices à afficher
indices = ['EUROSTOXX50', 'FTSE100', 'MIB', 'NIKKEI', 'SENSEX']

def math_date_to_real_date(math_date):
    # Conversion de la première date d'option en format datetime

    first_option_dates = datetime.strptime(dataf._first_option_dates[0], "%d-%m-%Y")
    index_ligne = df.index.get_loc(first_option_dates)
    index_ligne = index_ligne + math_date
    date = df['Date'][index_ligne]
    # Calcul de la vraie date en ajoutant le nombre de jours écoulés depuis la première date d'option
    return date


# Callback pour afficher les courbes lorsque le bouton est cliqué
@app.callback(Output('graph-container', 'children'),
              [Input('afficher-courbes-button', 'n_clicks')],
              [Input('date-picker-range', 'start_date'),
               Input('date-picker-range', 'end_date')])
def afficher_courbes(n_clicks, start_date, end_date):
    if n_clicks:

        # Convertir les dates en objets datetime
        start_date = datetime.strptime(start_date, '%Y-%m-%d')
        end_date = datetime.strptime(end_date, '%Y-%m-%d')

        # Filtrer les données en fonction des dates sélectionnées
        filtered_df = df[(df['Date'] >= start_date) & (df['Date'] <= end_date)]

        # Créer une trace pour les indices spécifiés
        traces = []
        for index in indices:
            if index in filtered_df.columns:
                trace = go.Scatter(
                    x=filtered_df['Date'],
                    y=filtered_df[index],
                    mode='lines+markers',
                    name=index
                )
                traces.append(trace)

        return dcc.Graph(
            id='graph',
            figure={
                'data': traces,
                'layout': {
                    'title': 'Courbes des indices',
                    'xaxis': {'title': 'Date'},
                    'yaxis': {'title': 'Valeur'}
                }
            }
        )

def gestion_accueil_layout():
    return html.Div(children=[
        navbar_layout(),  # Ajout de la barre de navigation
        html.Div(children=[
            html.H1(children='Page de Gestion de Portefeuille', className='mt-3'),
            
            # Ajouter un formulaire pour choisir les dates de début et de fin
            # html.Div([
            #     dcc.DatePickerRange(
            #         id='date-picker-range', 
            #         start_date=df['Date'].min(),
            #         end_date=DATE,
            #         display_format='DD/MM/YYYY',
            #         className='mt-3'
                 
            html.Button('Réinitialiser', id='reset-button', className='btn btn-primary mt-3',n_clicks=0),
            #     #html.Button('Afficher les courbes', id='afficher-courbes-button', className='btn btn-primary mt-3'),
            html.Button('Prévisualisation', id='preview-button', className='btn btn-secondary mt-3', n_clicks=0),
            html.Div(id="reset-output")
            ,
            
            # Ajouter un autre formulaire pour choisir une seule date
            html.Div([
                dcc.DatePickerSingle(
                    id='single-date-picker', 
                    date=DATE,
                    display_format='DD/MM/YYYY',
                    className='mt-3'
                ),
            ]),
            html.Div(id = 'date-value'),
            
            # Div pour afficher les courbes (initiallement vide)
            html.Div(id='graph-container'),

            # Ajouter la DataTable pour afficher les valeurs du fichier sortie.json
            html.Div(id='table-container')
        ], className='content')
    ])

@app.callback(
    Output('table-container', 'children'),
    [Input('preview-button', 'n_clicks')]
)
def display_table(n_clicks):
    if n_clicks:
        try:
            stringdate = DATE.strftime('%d-%m-%Y')
            # Exécuter le programme externe
            print(Listposition)
            len_previousrep = len(Listposition)
            print(len_previousrep)
            if len_previousrep == 0:
                previous_rep = {}
            else:
                previous_rep = Listposition[len_previousrep-1]
            currentpos = dp.pay_dividend_and_rebalance(stringdate,OPTIONNUMBER,previous_rep)
            Listposition.append(currentpos)
            print(currentpos)
            # Charger les données à partir de sortie.json
            assets = ["EUROSTOXX50", "MIB", "FTSE100", "NIKKEI", "SENSEX", "RGBP", "RJPY", "RINR"]
            # Extraire les données nécessaires
            date = currentpos['date']
            deltas = []
            for elem in assets:
                deltas.append(currentpos[elem])
            price = currentpos['price']

            # Charger les données à partir de output.json
            with open('./output.json', 'r') as f_output:
                data_output = json.load(f_output)
                asset_values = data_output['Past'][-1]

            # Créer une liste pour stocker les données de la table
            table_data = []

            # Extraire les noms des actifs
            

            # Parcourir les données et ajouter à table_data
            for i in range(len(assets)):
                entry = {
                    'Date': date,
                    'Asset': assets[i],
                    'Asset Value': asset_values[i],
                    'Delta': deltas[i],
                    'Price': '///'
                }
                table_data.append(entry)
            table_data.append({
                    'Date': date,
                    'Asset': '///',
                    'Asset Value': '///',
                    'Delta': '///',
                    'Price': price
                })

            # Créer le DataFrame pour la table
            df_table = pd.DataFrame(table_data)

            return dash_table.DataTable(
                id='table',
                columns=[
                    {'name': 'Date', 'id': 'Date'},
                    {'name': 'Asset', 'id': 'Asset'},
                    {'name': 'Asset Value', 'id': 'Asset Value'},
                    {'name': 'Delta', 'id': 'Delta'},
                    {'name': 'Price', 'id': 'Price'},
                    
                ],
                data=df_table.to_dict('records'),
                page_size=10
            )
        except FileNotFoundError:
            return "Le fichier sortie.json ou output.json n'a pas été trouvé."
        except Exception as e:
            return str(e)

# Callback pour réinitialiser le contenu du fichier de sortie
@app.callback(
    Output('reset-output', 'children'),
    [Input('reset-button', 'n_clicks')]
)
def reset_output_file(n_clicks):
    print("Inspi d'ailleurs, Marseille c'est the Wire")
    if n_clicks:
        try:
            # Supprimer le contenu du fichier de sortie
            with open('./sortie.json', 'w') as f:
                f.write('')

            DATE = datetime(2000,7,6)
            return ""
        except Exception as e:
            return str(e)
            
# Définir la mise en page de l'accueil
def accueil_layout():
    return html.Div(children=[
        navbar_layout(),
        html.Div(children=[
            html.H1(children='Mon Application Dash', className='mt-3'),
            html.Div(children='''
                Bienvenue sur ma première application Dash !
            ''', className='description'), 
            html.Div(id='output-container-button', className='mt-3')
        ], className='content')
    ])

# Définir la barre de navigation commune
def navbar_layout():
    return html.Nav(className='navbar navbar-expand-lg navbar-light bg-light', children=[
            html.A('Accueil', href='/accueil', className='navbar-brand'),
            html.A('Gestion de Portefeuille', href='/gestion', className='navbar-brand'),
        ], style={'marginBottom': 0, 'marginTop': 0, 'paddingBottom': 0, 'paddingTop': 0})

@app.callback(
    Output('date-value', 'children'),
    [Input('single-date-picker', 'date')]
)
def update_date_value(date):
    global DATE
    DATE = datetime.strptime(date, '%Y-%m-%d')
    return ""

# Définir la mise en page de l'application
app.layout = html.Div([
    dcc.Location(id='url', refresh=False),
    html.Div(id='page-content')
])


# Callback pour afficher la page en fonction de l'URL
@app.callback(Output('page-content', 'children'),
              [Input('url', 'pathname')])
def display_page(pathname):
    if pathname == '/gestion':
        return gestion_accueil_layout()
    if pathname == '/accueil':
        return accueil_layout()
    else:
        return accueil_layout()  # Par défaut, afficher la page d'accueil

# Démarrer le serveur
if __name__ == '__main__':
    app.run_server(debug=False)
    reset_output_file(1)
