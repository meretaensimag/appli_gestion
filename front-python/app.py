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

# Charger les données depuis les fichiers CSV
df1 = pd.read_csv('../data/ClosePrice.csv')
df2 = pd.read_csv('../data/XFORPrice.csv')
df3 = pd.read_csv('../data/XFORRet.csv')
df4 = pd.read_csv('../data/TauxInteret.csv')
df5 = pd.read_csv('../data/CloseRet.csv')

# Convertir les colonnes de dates en objets datetime pour tous les DataFrames

#df1['Date'] = pd.to_datetime(df1['Date'])
#df2['Date'] = pd.to_datetime(df2['Date'])
#df3['Date'] = pd.to_datetime(df3['Date'])
#df4['Date'] = pd.to_datetime(df4['Date'])
#df5['Date'] = pd.to_datetime(df5['Date'])


# Définir la mise en page de la gestion de portefeuille
def gestion_acceuil_layout():
    return html.Div(children=[
        navbar_layout(),
        html.Div(children=[
            html.H1(children='Pagegestiont', id='button-xforret', className='btn btn-primary mt-3 mr-3'),
                html.Button('TauxInteret', id='button-tauxinteret', className='btn btn-primary mt-3 mr-3'),
                html.Button('CloseRet', id='button-closeret', className='btn btn-primary mt-3 mr-3')
            ]),
            
            # Ajouter un formulaire pour choisir les dates de début et de fin
            html.Div([
                dcc.DatePickerRange(
                    id='date-picker-range',
                    start_date=df1['Date'].min(),
                    end_date=df1['Date'].max(),
                    display_format='DD/MM/YYYY',
                    className='mt-3'
                ),
                html.Button('Afficher les courbes', id='afficher-courbes-button', className='btn btn-primary mt-3')
            ]),
            
            # Div pour afficher les courbes (initiallement vide)
            html.Div(id='graph-container')
        ], className='content')
    

# Callback pour afficher les courbes lorsque le bouton est cliqué
@app.callback(Output('graph-container', 'children'),
              [Input('afficher-courbes-button', 'n_clicks')],
              [Input('date-picker-range', 'start_date'),
               Input('date-picker-range', 'end_date'),
               Input('button-closeprice', 'n_clicks'),
               Input('button-xforprice', 'n_clicks'),
               Input('button-xforret', 'n_clicks'),
               Input('button-tauxinteret', 'n_clicks'),
               Input('button-closeret', 'n_clicks')])
def afficher_courbes(n_clicks, start_date, end_date, btn_closeprice, btn_xforprice, btn_xforret, btn_tauxinteret, btn_closeret):
    if n_clicks:
        # Convertir les dates en objets datetime
        start_date = datetime.strptime(start_date, '%Y-%m-%d')
        end_date = datetime.strptime(end_date, '%Y-%m-%d')

        # Déterminer quel bouton a été cliqué
        clicked_id = dash.callback_context.triggered[0]['prop_id'].split('.')[0]

        # Sélectionner le DataFrame correspondant au bouton cliqué
        if clicked_id == 'button-closeprice':
            df = df1
        elif clicked_id == 'button-xforprice':
            df = df2
        elif clicked_id == 'button-xforret':
            df = df3
        elif clicked_id == 'button-tauxinteret':
            df = df4
        elif clicked_id == 'button-closeret':
            df = df5
        else:
            # Par défaut, utiliser df1
            df = df1

        # Filtrer les données en fonction des dates sélectionnées
        filtered_df = df[(df['Date'] >= start_date) & (df['Date'] <= end_date)]

        # Créer une trace pour les colonnes disponibles dans le DataFrame
        traces = []
        for column in filtered_df.columns:
            if column != 'Date':
                trace = go.Scatter(
                    x=filtered_df['Date'],
                    y=filtered_df[column],
                    mode='lines+markers',
                    name=column
                )
                traces.append(trace)

        return dcc.Graph(
            id='graph',
            figure={
                'data': traces,
                'layout': {
                    'title': f'Courbes des données de {clicked_id}',
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
            html.A('Accueil', href='', className='navbar-brand'),
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
    if pathname == '':
        return gestion_acceuil_layout()
    else:
        return accueil_layout()
# Démarrer le serveur
if __name__ == '__main__':
    app.run_server(debug=True)
