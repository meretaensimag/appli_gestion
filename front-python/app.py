import dash
from dash import dcc, html
from dash.dependencies import Input, Output
import subprocess

# Définir les feuilles de style externes
external_stylesheets = ['https://stackpath.bootstrapcdn.com/bootstrap/4.5.2/css/bootstrap.min.css', './assets/styles.css']

# Créer l'application Dash      
app = dash.Dash(__name__, external_stylesheets=external_stylesheets)

# Définir la mise en page de votre application avec une barre de navigation
app.layout = html.Div( children=[
    html.Nav(className='navbar fixed-top',children=[
        html.A('Accueil', href='#'),
        html.A('À propos', href='#'),
        html.A('Contact', href='#'),
    ]),
    html.Div(className='background-image', children=[
        html.Div(className='centered', children=[
            html.H1(children='Mon Application Dash', className='text-center'),
            html.Div(children='''
                Bienvenue sur ma première application Dash !
            ''', className='description text-center'), 
            html.Button('Appeler Fonction C++', id='button-call-cpp', className='btn btn-primary btn-lg mt-3')
        ])
    ])
])

# Définir la fonction pour appeler le programme C++
def call_cpp_function():
    subprocess.call(['path_to_your_cpp_program', 'arg1', 'arg2'])  # Remplacez path_to_your_cpp_program et les arguments nécessaires

# Associer la fonction d'appel de la fonction C++ au bouton
@app.callback(
    Output('output-container-button', 'children'),
    [Input('button-call-cpp', 'n_clicks')]
)
def update_output(n_clicks):
    if n_clicks is None:
        return ''
    else:
        call_cpp_function()
        return 'Fonction C++ appelée avec succès!'

# Démarrer le serveur
if __name__ == '__main__':
    app.run_server(debug=True)
