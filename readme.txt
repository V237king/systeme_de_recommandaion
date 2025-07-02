# Système de Recommandation

## Description du Projet
Ce projet développe un système de recommandation pour utilisateur basé sur trois algorithmes : 
- **KNN (k-plus proches voisins)** : Utilise la similarité entre utilisateurs pour prédire les préférences.
- **Matrix Factorization (MF)** : Décompose une matrice utilisateur-item pour estimer les notes.
- **PageRank (PAGERANK)** : Adapte l'algorithme de classement pour recommander des items populaires.

Le système comprend un serveur (`test_server_client`) et un client (`client`) qui communiquent via une socket. Les données d'interaction entre utilisateurs et items sont stockées dans des fichiers texte.

## Prérequis
- Un compilateur GCC installé sur votre système.
- La bibliothèque mathématique standard (liée avec `-lm`).
- Les fichiers de données dans le dossier `data/` :
  - `ratings.txt` : Toutes les interactions.
  - `train_ratings.txt` : Données pour l'entraînement (70 %).
  - `test_ratings.txt` : Données pour l'évaluation (30 %).

## Installation
1. Clonez le dépôt ou décompressez les fichiers sources dans un répertoire de travail.
2. Assurez-vous que le dossier `data/` contient les fichiers de données (voir section "data").
3. Suivez les étapes de compilation ci-dessous.

## Compilation
Les algorithmes sont compilés en bibliothèques (statiques ou dynamiques) pour une modularité. Choisissez une option :

### Option 1 : Bibliothèque Statique
Une bibliothèque statique intègre tout le code dans l'exécutable final.
- Étape 1 : Compiler les fichiers objets des algorithmes :
  ```bash
  gcc -c src/knn.c -o src/knn.o -Iinclude
  gcc -c src/matrix_factorisation.c -o src/matrix_factorisation.o -Iinclude
  gcc -c src/page_rank_recommendation.c -o src/page_rank_recommendation.o -Iinclude

-  Étape 2 : Créer la bibliothèque statique :
bash
ar rcs librecommend.a src/knn.o src/matrix_factorisation.o src/page_rank_recommendation.o



 -   Étape 3 : Compiler le serveur et le client :
bash

    gcc -c src/data_reader.c -o src/data_reader.o -Iinclude
    gcc -c src/server.c -o src/server.o -Iinclude
    gcc -c src/client.c -o src/client.o -Iinclude
    gcc -c tests/test_server_client.c -o tests/test_server_client.o -Iinclude
    gcc tests/test_server_client.o src/data_reader.o src/server.o src/client.o -L. -lrecommend -o tests/test_server_client -lm
    gcc src/data_reader.o src/client.o -L. -lrecommend -o client -lm

###Option 2 : Bibliothèque Dynamique

Une bibliothèque dynamique charge les algorithmes au runtime.

 -   Étape 1 : Compiler les objets avec code position indépendant :
    bash

gcc -fPIC -c src/knn.c -o src/knn.o -Iinclude
gcc -fPIC -c src/matrix_factorisation.c -o src/matrix_factorisation.o -Iinclude
gcc -fPIC -c src/page_rank_recommendation.c -o src/page_rank_recommendation.o -Iinclude


 -   Étape 2 : Créer la bibliothèque dynamique :
bash
gcc -shared -o librecommend.so src/knn.o src/matrix_factorisation.o src/page_rank_recommendation.o -lm


 -   Étape 3 : Compiler le serveur et le client :
bash

    gcc -c src/data_reader.c -o src/data_reader.o -Iinclude
    gcc -c src/server.c -o src/server.o -Iinclude
    gcc -c src/client.c -o src/client.o -Iinclude
    gcc -c tests/test_server_client.c -o tests/test_server_client.o -Iinclude
    gcc tests/test_server_client.o src/data_reader.o src/server.o src/client.o -L. -lrecommend -Wl,-rpath=. -o tests/test_server_client -lm
    gcc src/data_reader.o src/client.o -L. -lrecommend -Wl,-rpath=. -o client -lm



#######Utilisation###########

    Lancez le serveur dans un terminal :
    bash

./tests/test_server_client


Dans un autre terminal, testez un algorithme. Exemple pour KNN avec l'utilisateur 1 :
bash

    ./client 127.0.0.1 8080 KNN 1
        Remplacez KNN par MF ou PR pour tester les autres algorithmes.
        Le client retournera une liste comme item_id=score,item_id=score,....

####Structure des Fichiers####

    src/ : Contient les implémentations :
        knn.c : Algorithme KNN.
        matrix_factorisation.c : Algorithme MF.
        page_rank_recommendation.c : Algorithme PR.
        server.c : Gère les requêtes et appelle les algorithmes.
        client.c : Envoie des requêtes au serveur.
        data_reader.c : Lit les fichiers de données.
    include/ : En-têtes des fonctions (ex. knn.h).
    data/ : Fichiers de données avec des exemples fournis.
    tests/ : Contient test_server_client.c pour tester le serveur.

#######Données#######

Les fichiers dans data/ doivent suivre ce format :

    Exemple de ligne : 1,101,1,4.0,1727712001 (user_id, item_id, timestamp, rating, date).
    train_ratings.txt : Utilisé pour entraîner les modèles.
    test_ratings.txt : Utilisé pour évaluer les prédictions.