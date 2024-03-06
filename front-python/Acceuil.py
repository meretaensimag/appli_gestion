import dash
from dash import dcc 
from dash import html

# Définir les feuilles de style externes
external_stylesheets = ['https://stackpath.bootstrapcdn.com/bootstrap/4.5.2/css/bootstrap.min.css', './styles.css']

# Créer l'application Dash      
app = dash.Dash(__name__, external_stylesheets=external_stylesheets)

# Définir la mise en page de votre application avec une barre de navigation
app.layout = html.Div(children=[
    html.Nav(children=[
        html.A('Accueil', href='#'),
        html.A('À propos', href='#'),
        html.A('Contact', href='#'),
        # Ajoutez d'autres liens de navigation selon vos besoins
    ]),
    html.Div(children=[
        html.H1(children='Mon Application Dash'),
        html.Div(children='''
            Bienvenue sur ma première application Dash !
        ''', className='description'),  # Utilisation de la classe CSS dans la div
        # Ajoutez d'autres composants Dash selon vos besoins
    ], className='content')
])

# Ajouter des callbacks si nécessaire
# ...

# Démarrer le serveur
if __name__ == '__main__':
    app.run_server(debug=True)
 