#ifndef MATRIX_FACTORISATION_H
#define MATRIX_FACTORISATION_H

#include "data_reader.h"

// Structure pour stocker le modèle de factorisation de matrices
typedef struct {
    float** user_factors; // Matrice P (utilisateurs × facteurs)
    float** item_factors; // Matrice Q (items × facteurs)
    int num_users;
    int num_items;
    int num_factors;
} MatrixFactorisationModel;

// Initialise le modèle de factorisation de matrices
// Paramètres : num_users (nombre d'utilisateurs), num_items (nombre d'items),
//              num_factors (nombre de facteurs latents)
// Retour : pointeur vers le modèle, ou NULL si erreur
MatrixFactorisationModel* init_matrix_factorisation(int num_users, int num_items, int num_factors);

// Entraîne le modèle avec la descente de gradient stochastique
// Paramètres : model (modèle à entraîner), transactions (données d'entraînement),
//              count (nombre de transactions), learning_rate (taux d'apprentissage),
//              reg (paramètre de régularisation), num_iterations (nombre d'itérations)
// Retour : 0 si succès, -1 si erreur
int train_matrix_factorisation(MatrixFactorisationModel* model, Transaction* transactions,
                               int count, float learning_rate, float reg, int num_iterations);

// Prédit un rating pour une paire utilisateur-item
// Paramètres : model (modèle entraîné), user_id, item_id
// Retour : rating prédit (entre 1.0 et 5.0)
float predict_rating(MatrixFactorisationModel* model, int user_id, int item_id);

// Calcule les recommandations pour un utilisateur
// Paramètres : model (modèle entraîné), transactions (données d'entraînement),
//              count (nombre de transactions), user_id (utilisateur cible),
//              num_recommendations (nombre de recommandations),
//              recommendations (tableau de sortie)
// Retour : 0 si succès, -1 si erreur
int compute_mf_recommendations(MatrixFactorisationModel* model, Transaction* transactions,
                               int count, int user_id, int num_recommendations,
                               Recommendation* recommendations);

// Calcule les métriques RMSE et MAE pour Matrix Factorisation
// Paramètres : model (modèle entraîné), test_transactions (données de test),
//              test_count (nombre de transactions de test), rmse (pointeur pour RMSE),
//              mae (pointeur pour MAE)
// Retour : 0 si succès, -1 si erreur
int compute_mf_metrics(MatrixFactorisationModel* model, Transaction* test_transactions,
                       int test_count, float* rmse, float* mae);

// Libère la mémoire du modèle
void free_matrix_factorisation(MatrixFactorisationModel* model);

#endif