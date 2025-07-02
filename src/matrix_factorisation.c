#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include "./../include/matrix_factorisation.h"

// Initialise le modèle avec des valeurs aléatoires
MatrixFactorisationModel* init_matrix_factorisation(int num_users, int num_items, int num_factors) {
    MatrixFactorisationModel* model = (MatrixFactorisationModel*)malloc(sizeof(MatrixFactorisationModel));
    if (model == NULL) {
        fprintf(stderr, "Erreur d'allocation mémoire pour le modèle\n");
        return NULL;
    }

    model->num_users = num_users;
    model->num_items = num_items;
    model->num_factors = num_factors;

    // Allouer la matrice user_factors
    model->user_factors = (float**)malloc(num_users * sizeof(float*));
    if (model->user_factors == NULL) {
        fprintf(stderr, "Erreur d'allocation mémoire pour user_factors\n");
        free(model);
        return NULL;
    }
    for (int i = 0; i < num_users; i++) {
        model->user_factors[i] = (float*)malloc(num_factors * sizeof(float));
        if (model->user_factors[i] == NULL) {
            fprintf(stderr, "Erreur d'allocation mémoire pour user_factors[%d]\n", i);
            for (int j = 0; j < i; j++) free(model->user_factors[j]);
            free(model->user_factors);
            free(model);
            return NULL;
        }
        // Initialiser avec des valeurs aléatoires entre 0 et 1
        for (int f = 0; f < num_factors; f++) {
            model->user_factors[i][f] = (float)rand() / RAND_MAX;
        }
    }

    // Allouer la matrice item_factors
    model->item_factors = (float**)malloc(num_items * sizeof(float*));
    if (model->item_factors == NULL) {
        fprintf(stderr, "Erreur d'allocation mémoire pour item_factors\n");
        for (int i = 0; i < num_users; i++) free(model->user_factors[i]);
        free(model->user_factors);
        free(model);
        return NULL;
    }
    for (int i = 0; i < num_items; i++) {
        model->item_factors[i] = (float*)malloc(num_factors * sizeof(float));
        if (model->item_factors[i] == NULL) {
            fprintf(stderr, "Erreur d'allocation mémoire pour item_factors[%d]\n", i);
            for (int j = 0; j < i; j++) free(model->item_factors[j]);
            for (int j = 0; j < num_users; j++) free(model->user_factors[j]);
            free(model->item_factors);
            free(model->user_factors);
            free(model);
            return NULL;
        }
        // Initialiser avec des valeurs aléatoires entre 0 et 1
        for (int f = 0; f < num_factors; f++) {
            model->item_factors[i][f] = (float)rand() / RAND_MAX;
        }
    }

    return model;
}

int train_matrix_factorisation(MatrixFactorisationModel* model, Transaction* transactions,
                               int count, float learning_rate, float reg, int num_iterations) {
    if (model == NULL || transactions == NULL) {
        fprintf(stderr, "Erreur : modèle ou transactions NULL\n");
        return -1;
    }

    // Descente de gradient stochastique
    for (int iter = 0; iter < num_iterations; iter++) {
        for (int i = 0; i < count; i++) {
            int user_id = transactions[i].user_id - 1; // Ajuster pour index 0
            int item_id = transactions[i].item_id - 1; // Ajuster pour index 0
            float rating = transactions[i].rating;

            // Vérifier les indices
            if (user_id >= model->num_users || item_id >= model->num_items) {
                fprintf(stderr, "Erreur : user_id=%d ou item_id=%d hors limites\n",
                        user_id + 1, item_id + 1);
                continue;
            }

            // Calculer le rating prédit
            float predicted = 0;
            for (int f = 0; f < model->num_factors; f++) {
                predicted += model->user_factors[user_id][f] * model->item_factors[item_id][f];
            }

            // Calculer l'erreur
            float error = rating - predicted;

            // Mettre à jour les facteurs
            for (int f = 0; f < model->num_factors; f++) {
                float user_factor = model->user_factors[user_id][f];
                float item_factor = model->item_factors[item_id][f];

                model->user_factors[user_id][f] += learning_rate * 
                    (error * item_factor - reg * user_factor);
                model->item_factors[item_id][f] += learning_rate * 
                    (error * user_factor - reg * item_factor);
            }
        }
    }

    return 0;
}

float predict_rating(MatrixFactorisationModel* model, int user_id, int item_id) {
    user_id--; // Ajuster pour index 0
    item_id--; // Ajuster pour index 0

    if (user_id >= model->num_users || item_id >= model->num_items || user_id < 0 || item_id < 0) {
        return 0; // Retourner 0 si hors limites
    }

    float predicted = 0;
    for (int f = 0; f < model->num_factors; f++) {
        predicted += model->user_factors[user_id][f] * model->item_factors[item_id][f];
    }

    // Limiter à l'intervalle [1.0, 5.0]
    if (predicted < 1.0) predicted = 1.0;
    if (predicted > 5.0) predicted = 5.0;
    return predicted;
}

int compute_mf_recommendations(MatrixFactorisationModel* model, Transaction* transactions,
                               int count, int user_id, int num_recommendations,
                               Recommendation* recommendations) {
    // Vérifier si l'utilisateur existe
    int user_exists = 0;
    int max_item_id = 0;
    for (int i = 0; i < count; i++) {
        if (transactions[i].user_id == user_id) {
            user_exists = 1;
        }
        if (transactions[i].item_id > max_item_id) {
            max_item_id = transactions[i].item_id;
        }
    }
    if (!user_exists) {
        fprintf(stderr, "Erreur : user_id %d n'existe pas dans les données\n", user_id);
        return -1;
    }

    // Initialiser les recommandations
    for (int i = 0; i < num_recommendations; i++) {
        recommendations[i].item_id = 0;
        recommendations[i].score = 0;
    }

    // Prédire les scores pour tous les items non notés
    float* predicted_scores = (float*)calloc(max_item_id + 1, sizeof(float));
    if (predicted_scores == NULL) {
        fprintf(stderr, "Erreur d'allocation mémoire pour predicted_scores\n");
        return -1;
    }

    for (int item_id = 1; item_id <= max_item_id; item_id++) {
        // Vérifier si l'utilisateur n'a pas noté cet item
        int rated = 0;
        for (int i = 0; i < count; i++) {
            if (transactions[i].user_id == user_id && transactions[i].item_id == item_id) {
                rated = 1;
                break;
            }
        }
        if (!rated) {
            predicted_scores[item_id] = predict_rating(model, user_id, item_id);
            // Insérer dans les recommandations si le score est parmi les plus élevés
            for (int i = 0; i < num_recommendations; i++) {
                if (predicted_scores[item_id] > recommendations[i].score) {
                    for (int j = num_recommendations - 1; j > i; j--) {
                        recommendations[j] = recommendations[j - 1];
                    }
                    recommendations[i].item_id = item_id;
                    recommendations[i].score = predicted_scores[item_id];
                    break;
                }
            }
        }
    }

    free(predicted_scores);
    return 0;
}

int compute_mf_metrics(MatrixFactorisationModel* model, Transaction* test_transactions,
                       int test_count, float* rmse, float* mae) {
    float sum_squared_error = 0;
    float sum_absolute_error = 0;
    int valid_predictions = 0;

    for (int i = 0; i < test_count; i++) {
        int user_id = test_transactions[i].user_id;
        int item_id = test_transactions[i].item_id;
        float actual_rating = test_transactions[i].rating;

        float predicted_rating = predict_rating(model, user_id, item_id);
        if (predicted_rating > 0) { // Prédiction valide
            float error = predicted_rating - actual_rating;
            sum_squared_error += error * error;
            sum_absolute_error += fabs(error);
            valid_predictions++;
        }
    }

    if (valid_predictions == 0) {
        fprintf(stderr, "Erreur : aucune prédiction valide pour les métriques\n");
        return -1;
    }

    *rmse = sqrt(sum_squared_error / valid_predictions);
    *mae = sum_absolute_error / valid_predictions;
    return 0;
}

void free_matrix_factorisation(MatrixFactorisationModel* model) {
    if (model == NULL) return;
    for (int i = 0; i < model->num_users; i++) {
        free(model->user_factors[i]);
    }
    for (int i = 0; i < model->num_items; i++) {
        free(model->item_factors[i]);
    }
    free(model->user_factors);
    free(model->item_factors);
    free(model);
}