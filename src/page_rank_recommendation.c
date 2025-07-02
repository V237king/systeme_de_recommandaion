#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include "./../include/page_rank_recommendation.h"

#define DAMPING_FACTOR 0.85
#define ITERATIONS 10

PageRankModel* init_page_rank_model(Transaction* transactions, int count) {
    if (transactions == NULL || count == 0) {
        fprintf(stderr, "Erreur : transactions NULL ou count=0\n");
        return NULL;
    }

    // Trouver le nombre max d'utilisateurs et d'items
    int max_user_id = 0, max_item_id = 0;
    for (int i = 0; i < count; i++) {
        if (transactions[i].user_id > max_user_id) max_user_id = transactions[i].user_id;
        if (transactions[i].item_id > max_item_id) max_item_id = transactions[i].item_id;
    }

    PageRankModel* model = (PageRankModel*)malloc(sizeof(PageRankModel));
    if (model == NULL) {
        fprintf(stderr, "Erreur d'allocation mémoire pour le modèle\n");
        return NULL;
    }

    model->num_users = max_user_id;
    model->num_items = max_item_id;

    // Allouer la matrice d'adjacence
    model->adjacency_matrix = (float**)malloc(model->num_users * sizeof(float*));
    if (model->adjacency_matrix == NULL) {
        fprintf(stderr, "Erreur d'allocation mémoire pour la matrice d'adjacence\n");
        free(model);
        return NULL;
    }
    for (int i = 0; i < model->num_users; i++) {
        model->adjacency_matrix[i] = (float*)calloc(model->num_items, sizeof(float));
        if (model->adjacency_matrix[i] == NULL) {
            fprintf(stderr, "Erreur d'allocation mémoire pour la ligne %d\n", i);
            for (int j = 0; j < i; j++) free(model->adjacency_matrix[j]);
            free(model->adjacency_matrix);
            free(model);
            return NULL;
        }
    }

    // Remplir la matrice d'adjacence (normalisation des ratings entre 0 et 1)
    for (int i = 0; i < count; i++) {
        int user_idx = transactions[i].user_id - 1; // Index 0-based
        int item_idx = transactions[i].item_id - 1;
        float rating = transactions[i].rating / 5.0; // Normalisation (max rating = 5.0)
        model->adjacency_matrix[user_idx][item_idx] = rating;
    }

    return model;
}

int compute_page_rank_recommendations(PageRankModel* model, Transaction* transactions,
                                      int count, int user_id, int num_recommendations,
                                      Recommendation* recommendations) {
    if (model == NULL || user_id < 1 || user_id > model->num_users) {
        fprintf(stderr, "Erreur : modèle NULL ou user_id invalide\n");
        return -1;
    }

    int user_idx = user_id - 1;
    // Initialiser les scores (vecteur PageRank)
    float* scores = (float*)calloc(model->num_items, sizeof(float));
    if (scores == NULL) {
        fprintf(stderr, "Erreur d'allocation mémoire pour scores\n");
        return -1;
    }

    // Initialisation : score de 1 pour l'utilisateur, propagation aux items
    for (int iter = 0; iter < ITERATIONS; iter++) {
        float* new_scores = (float*)calloc(model->num_items, sizeof(float));
        if (new_scores == NULL) {
            fprintf(stderr, "Erreur d'allocation mémoire pour new_scores\n");
            free(scores);
            return -1;
        }

        for (int i = 0; i < model->num_items; i++) {
            float sum = 0.0;
            for (int j = 0; j < model->num_users; j++) {
                if (model->adjacency_matrix[j][i] > 0) {
                    sum += scores[i] * model->adjacency_matrix[j][i]; // Propagation simplifiée
                }
            }
            new_scores[i] = (1.0 - DAMPING_FACTOR) / model->num_items + DAMPING_FACTOR * sum;
        }

        free(scores);
        scores = new_scores;
    }

    // Récupérer les meilleures recommandations (items non notés par l'utilisateur)
    for (int i = 0; i < num_recommendations; i++) {
        recommendations[i].item_id = 0;
        recommendations[i].score = 0;
    }

    int* rated_items = (int*)calloc(model->num_items + 1, sizeof(int));
    if (rated_items == NULL) {
        fprintf(stderr, "Erreur d'allocation mémoire pour rated_items\n");
        free(scores);
        return -1;
    }
    for (int i = 0; i < count; i++) {
        if (transactions[i].user_id == user_id) {
            rated_items[transactions[i].item_id] = 1;
        }
    }

    for (int item_id = 1; item_id <= model->num_items; item_id++) {
        int item_idx = item_id - 1;
        if (!rated_items[item_id] && scores[item_idx] > 0) {
            for (int i = 0; i < num_recommendations; i++) {
                if (scores[item_idx] > recommendations[i].score) {
                    for (int j = num_recommendations - 1; j > i; j--) {
                        recommendations[j] = recommendations[j - 1];
                    }
                    recommendations[i].item_id = item_id;
                    recommendations[i].score = scores[item_idx];
                    break;
                }
            }
        }
    }

    free(scores);
    free(rated_items);
    return 0;
}

void free_page_rank_model(PageRankModel* model) {
    if (model == NULL) return;
    for (int i = 0; i < model->num_users; i++) {
        free(model->adjacency_matrix[i]);
    }
    free(model->adjacency_matrix);
    free(model);
}