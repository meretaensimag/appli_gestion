import dash
from dash import dcc, html, Input, Output
import pandas as pd
import plotly.graph_objs as go
from datetime import datetime
import PortfolioHandler as ph

# Définir les feuilles de style externes
external_stylesheets = ['https://stackpath.bootstrapcdn.com/bootstrap/4.5.2/css/bootstrap.min.css', './assets/styles.css']

# Créer l'application Dash      
app = dash.Dash(__name__, external_stylesheets=external_stylesheets)

# Charger les données depuis le fichier CSV
df = pd.read_csv('../data/ClosePrice.csv')  # Assurez-vous de remplacer "votre_fichier.csv" par le chemin de votre fichier CSV


# Convertir les colonnes de dates en objets datetime
df['Date'] = pd.to_datetime(df['Date'])

# Indices à afficher
indices = ['EUROSTOXX50', 'FTSE100', 'MIB', 'NIKKEI', 'SENSEX']

# Définir la mise en page de la gestion de portefeuille
def gestion_accueil_layout():
    return html.Div(children=[
        navbar_layout(),
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
                html.Button('Afficher les courbes', id='afficher-courbes-button', className='btn btn-primary mt-3')
            ]),
            # Div pour afficher les courbes (initiallement vide)
            html.Div(id='graph-container')
        ], className='content')
    ])

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

# Définir la mise en page de l'accueil
def accueil_layout():
    return html.Div(children=[
        navbar_layout(),
        html.Div(children=[
            html.H1(children='Mon Application Dash', className='mt-3'),
            html.Div(children='''
                Bienvenue sur ma première application Dash !
            ''', className='description'), 
            html.Button('Appeler Fonction C++', id='button-call-cpp', className='btn btn-primary mt-3 btn-sm'),
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
        return ph.gestion_portefeuille_layout()
    if pathname == '/accueil':
        return 
    else:
        return accueil_layout()

# Démarrer le serveur
if __name__ == '__main__':
    app.run_server(debug=True)