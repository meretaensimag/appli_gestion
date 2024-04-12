# Projet Appli_gestion

**Équipe**:
- Nada Labzae
- Pierre-Yves Marec
- Alan Meret

**Projet numéro 11**: Chroelia

## Comment lancer le projet:

### Compilation:
Pour exécuter le projet, il est nécessaire de compiler d'abord la partie du code C++.

1. Naviguez vers le dossier `src`.
2. Exécutez les commandes suivantes dans le terminal:

```bash
mkdir build
cd build
cmake -DCMAKE_PREFIX_PATH=/matieres/5MMPCPD/pnl ..
```
(*Remarque*: Remplacez `/matieres/5MMPCPD/pnl` par le chemin correspondant au fichier `pnlconfig.cmake`.)

3. Ensuite, exécutez la commande `make`.

Une fois ces étapes terminées, le code C++ devrait être compilé et prêt à être utilisé.

### Lancement de l'interface graphique:
Pour lancer l'interface graphique, suivez les étapes suivantes:

1. Accédez au dossier `front-python`.
2. Exécutez la commande suivante dans le terminal:

```bash
python app.py
```

Un serveur se lancera automatiquement sur le port spécifié dans l'invite de commande. Vous pourrez y accéder via le navigateur web de votre choix.