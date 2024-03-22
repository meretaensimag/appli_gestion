import dash
from dash import dcc, html, Input, Output
import pandas as pd
import plotly.graph_objs as go
from datetime import datetime
import subprocess
import data_provider as dp
import dash_table
import json

# Définir les feuilles de style externes
external_stylesheets = ['https://stackpath.bootstrapcdn.com/bootstrap/4.5.2/css/bootstrap.min.css', './assets/styles.css']

# Créer l'application Dash      
app = dash.Dash(__name__, external_stylesheets=external_stylesheets)

# Charger les données depuis le fichier CSV
df = pd.read_csv('../data/ClosePrice.csv') 

# Convertir les colonnes de dates en objets datetime
df['Date'] = pd.to_datetime(df['Date'])

# Indices à afficher
indices = ['EUROSTOXX50', 'FTSE100', 'MIB', 'NIKKEI', 'SENSEX']

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


# Callback pour afficher les courbes lorsque le bouton est cliqué
@app.callback(Output('preview-button', 'children'),
              [Input('preview-button', 'n_clicks')],
              [Input('date-picker-range', 'start_date'),
               Input('date-picker-range', 'end_date')])
def execute_command(n_clicks, start_date, end_date):
    if n_clicks:
        message = "Appel de la fonction C++"
        dp.generate_output_json("./output.json")        
        command = ["./hedging_portfolio", "../../front-python/output.json", "../../front-python/sortie.json"]
        subprocess.run(command, cwd="../src/build")
        return message + "\nPrévisualisation effectuée avec succès."

# Définir la mise en page de la gestion de portefeuille
def gestion_accueil_layout():
    return html.Div(children=[
        navbar_layout(),  # Ajout de la barre de navigation
        html.Div(children=[
            html.H1(children='Page de Gestion de Portefeuille', className='mt-3'),
            
            # Ajouter un formulaire pour choisir les dates de début et de fin
            html.Div([
                dcc.DatePickerRange(
                    id='date-picker-range',
                    start_date=df['Date'].min(),
                    end_date=df['Date'].max(),
                    display_format='DD/MM/YYYY',
                    className='mt-3'
                ),
                html.Button('Afficher les courbes', id='afficher-courbes-button', className='btn btn-primary mt-3'),
                html.Button('Prévisualisation', id='preview-button', className='btn btn-secondary mt-3 ml-2')
            ]),
            # Div pour afficher les courbes (initiallement vide)
            html.Div(id='graph-container'),

            # Ajouter la DataTable pour afficher les valeurs du fichier sortie.json
            html.Div(id='table-container')
        ], className='content')
    ])

@app.callback(Output('table-container', 'children'),
              [Input('preview-button', 'n_clicks')])
def display_table(n_clicks):
    if n_clicks:
        try:
            with open('./sortie.json', 'r') as f:
                data = json.load(f)
                return dash_table.DataTable(
                    id='table',
                    columns=[
                        {'name': 'Date', 'id': 'date'},
                        {'name': 'Deltas', 'id': 'deltas'},
                        {'name': 'Deltas StdDev', 'id': 'deltasStdDev'},
                        {'name': 'Price', 'id': 'price'},
                        {'name': 'Price StdDev', 'id': 'priceStdDev'}
                    ],
                    data=data,
                    page_size=10
                )
        except FileNotFoundError:
            return "Le fichier sortie.json n'a pas été trouvé."
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
            html.A('Contact', href='', className='navbar-brand'),
        ], style={'marginBottom': 0, 'marginTop': 0, 'paddingBottom': 0, 'paddingTop': 0})

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
