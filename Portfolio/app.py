import dash
from dash import html

# Créer une application Dash
app = dash.Dash(__name__)

app.layout = html.Div([
    html.H1('Gestion de Portefeuille'),
    html.Div([
        # Ici, vous pouvez ajouter plus de composants pour votre application...
        html.P('Bienvenue sur votre tableau de bord de gestion de portefeuille.'),
    ]),
])

if __name__ == '__main__':
    app.run_server(debug=True)
