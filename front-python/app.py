import dash
from dash import dcc, html

# Définir les feuilles de style externes
external_stylesheets = ['https://stackpath.bootstrapcdn.com/bootstrap/4.5.2/css/bootstrap.min.css', './assets/styles.css']

# Créer l'application Dash      
app = dash.Dash(__name__, external_stylesheets=external_stylesheets)

# Définir la mise en page de votre application avec une barre de navigation
app.layout = html.Div( children=[
    html.Nav(className='navbar',children=[
        html.A('Accueil', href='#'),
        html.A('À propos', href='#'),
        html.A('Contact', href='#'),
    ]),
    html.Div(children=[
        html.H1(children='Mon Application Dash'),
        html.Div(children='''
            Bienvenue sur ma première application Dash !
        ''', className='description'), 
    ], className='content')
])

# Démarrer le serveur
if __name__ == '__main__':
    app.run_server(debug=True)
