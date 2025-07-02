#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <string.h> // Pour strlen
#include <unistd.h> // Pour write
#include "./../include/knn.h"

// Structure pour stocker la similarité entre utilisateurs
typedef struct {
    int user_id;
    float similarity;
} UserSimilarity;

// Calcule la corrélation de Pearson entre deux utilisateurs
 float compute_pearson_correlation(Transaction* transactions, int count,
                                        int user1_id, int user2_id, int client_fd) {
    float sum1 = 0, sum2 = 0, sum1_sq = 0, sum2_sq = 0, sum_product = 0;
    int common_items = 0;

    // Parcourir une seule fois pour trouver les items communs
    for (int i = 0; i < count; i++) {
        if (transactions[i].user_id == user1_id) {
            for (int j = 0; j < count; j++) {
                if (transactions[j].user_id == user2_id && transactions[i].item_id == transactions[j].item_id) {
                    float rating1 = transactions[i].rating;
                    float rating2 = transactions[j].rating;
                    sum1 += rating1;
                    sum2 += rating2;
                    sum1_sq += rating1 * rating1;
                    sum2_sq += rating2 * rating2;
                    sum_product += rating1 * rating2;
                    common_items++;
                }
            }
        }
    }

    // Si moins de 2 items communs, signaler une erreur
    if (common_items < 2) {
        char response[256];
        snprintf(response, sizeof(response), "Pas assez d'items communs (%d) pour user_id %d et %d\n",
                 common_items, user1_id, user2_id);
        write(client_fd, response, strlen(response));
        return -1; // Échec
    }

    float mean1 = sum1 / common_items;
    float mean2 = sum2 / common_items;
    float numerator = sum_product - common_items * mean1 * mean2;
    float denominator = sqrt((sum1_sq - common_items * mean1 * mean1) *
                            (sum2_sq - common_items * mean2 * mean2));

    if (denominator == 0) {
        char response[256];
        snprintf(response, sizeof(response), "Dénominateur nul pour user_id %d et %d\n",
                 user1_id, user2_id);
        write(client_fd, response, strlen(response));
        return -1; // Échec
    }
    return numerator / denominator; // Retourne la similarité
}

int compute_knn_recommendations(Transaction* transactions, int count, int user_id,
                                int k, int num_recommendations, Recommendation* recommendations, int client_fd) {
    char debug[256];

    // Vérifier si l'utilisateur existe
    int user_exists = 0;
    for (int i = 0; i < count; i++) {
        if (transactions[i].user_id == user_id) {
            user_exists = 1;
            break;
        }
    }
    if (!user_exists) {
        char response[256];
        snprintf(response, sizeof(response), "Erreur : user_id %d n'existe pas\n", user_id);
        write(client_fd, response, strlen(response));
        return -1;
    }
    snprintf(debug, sizeof(debug), "Utilisateur %d existe, count=%d\n", user_id, count);
    write(client_fd, debug, strlen(debug));

    // Trouver le plus grand user_id
    int max_user_id = 0;
    for (int i = 0; i < count; i++) {
        if (transactions[i].user_id > max_user_id) {
            max_user_id = transactions[i].user_id;
        }
    }

    // Allouer un tableau pour les similarités
    UserSimilarity* similarities = (UserSimilarity*)malloc((max_user_id + 1) * sizeof(UserSimilarity));
    if (similarities == NULL) {
        char response[256];
        snprintf(response, sizeof(response), "Erreur d'allocation mémoire\n");
        write(client_fd, debug, strlen(response)); // Correction : strlen(response)
        return -1;
    }
    snprintf(debug, sizeof(debug), "Allocation similarities OK, max_user_id=%d\n", max_user_id);
    write(client_fd, debug, strlen(debug));

    int sim_count = 0;
    for (int other_user = 1; other_user <= max_user_id; other_user++) {
        if (other_user != user_id) {
            float sim = compute_pearson_correlation(transactions, count, user_id, other_user, client_fd);
            if (sim >= -1.0) {
                similarities[sim_count].user_id = other_user;
                similarities[sim_count].similarity = sim;
                sim_count++;
                snprintf(debug, sizeof(debug), "Similarité avec user %d: %.2f\n", other_user, sim);
                write(client_fd, debug, strlen(debug));
            }
        }
    }

    if (sim_count == 0) {
        char response[256];
        snprintf(response, sizeof(response), "Aucun voisin similaire pour user_id %d\n", user_id);
        write(client_fd, response, strlen(response));
        free(similarities);
        return -1;
    }
    snprintf(debug, sizeof(debug), "Nombre de voisins trouvés: %d\n", sim_count);
    write(client_fd, debug, strlen(debug));

    // Trier les similarités
    for (int i = 0; i < sim_count - 1; i++) {
        for (int j = i + 1; j < sim_count; j++) {
            if (similarities[i].similarity < similarities[j].similarity) {
                UserSimilarity temp = similarities[i];
                similarities[i] = similarities[j];
                similarities[j] = temp;
            }
        }
    }

    int actual_k = (sim_count < k) ? sim_count : k;
    if (actual_k == 0) {
        char response[256];
        snprintf(response, sizeof(response), "Aucun voisin pour k=%d\n", k);
        write(client_fd, response, strlen(response));
        free(similarities);
        return -1;
    }
    snprintf(debug, sizeof(debug), "k effectif: %d\n", actual_k);
    write(client_fd, debug, strlen(debug));

    // Trouver le plus grand item_id
    int max_item_id = 0;
    for (int i = 0; i < count; i++) {
        if (transactions[i].item_id > max_item_id) {
            max_item_id = transactions[i].item_id;
        }
    }

    // Initialiser les tableaux pour les prédictions
    float* predicted_scores = (float*)calloc(max_item_id + 1, sizeof(float));
    float* weights_sum = (float*)calloc(max_item_id + 1, sizeof(float));
    if (predicted_scores == NULL || weights_sum == NULL) {
        char response[256];
        snprintf(response, sizeof(response), "Erreur d'allocation pour les scores\n");
        write(client_fd, response, strlen(response));
        free(similarities);
        free(predicted_scores);
        free(weights_sum);
        return -1;
    }
    snprintf(debug, sizeof(debug), "Allocation scores OK, max_item_id=%d\n", max_item_id);
    write(client_fd, debug, strlen(debug));

    // Calculer les scores prédits
    for (int i = 0; i < actual_k; i++) {
        int neighbor_id = similarities[i].user_id;
        float sim = similarities[i].similarity;
        for (int j = 0; j < count; j++) {
            if (transactions[j].user_id == neighbor_id) {
                int item_id = transactions[j].item_id;
                int rated_by_user = 0;
                for (int m = 0; m < count; m++) {
                    if (transactions[m].user_id == user_id && transactions[m].item_id == item_id) {
                        rated_by_user = 1;
                        break;
                    }
                }
                if (!rated_by_user && weights_sum[item_id] >= 0) {
                    predicted_scores[item_id] += sim * transactions[j].rating;
                    weights_sum[item_id] += sim;
                }
            }
        }
    }
    snprintf(debug, sizeof(debug), "Prédictions calculées, vérifions weights_sum...\n");
    write(client_fd, debug, strlen(debug));

    // Remplir les recommandations
    for (int i = 0; i < num_recommendations; i++) {
        recommendations[i].item_id = 0;
        recommendations[i].score = 0;
    }

    for (int item_id = 1; item_id <= max_item_id; item_id++) {
        if (weights_sum[item_id] > 0) {
            float score = predicted_scores[item_id] / weights_sum[item_id];
            for (int i = 0; i < num_recommendations; i++) {
                if (score > recommendations[i].score || recommendations[i].item_id == 0) {
                    for (int j = num_recommendations - 1; j > i; j--) {
                        recommendations[j] = recommendations[j - 1];
                    }
                    recommendations[i].item_id = item_id;
                    recommendations[i].score = score;
                    break;
                }
            }
        }
    }
    snprintf(debug, sizeof(debug), "Recommandations remplies: %d, %d, %d\n",
             recommendations[0].item_id, recommendations[1].item_id, recommendations[2].item_id);
    write(client_fd, debug, strlen(debug));

    free(similarities);
    free(predicted_scores);
    free(weights_sum);
    return 0;
}

int compute_knn_metrics(Transaction* train_transactions, int train_count,
                        Transaction* test_transactions, int test_count,
                        int k, float* rmse, float* mae, int client_fd) {
    float sum_squared_error = 0;
    float sum_absolute_error = 0;
    int valid_predictions = 0;

    Recommendation* recommendations = (Recommendation*)malloc(test_count * sizeof(Recommendation));
    if (recommendations == NULL) {
        char response[256];
        snprintf(response, sizeof(response), "Erreur d'allocation mémoire\n");
        write(client_fd, response, strlen(response));
        return -1;
    }

    for (int i = 0; i < test_count; i++) {
        int user_id = test_transactions[i].user_id;
        int item_id = test_transactions[i].item_id;
        float actual_rating = test_transactions[i].rating;

        if (compute_knn_recommendations(train_transactions, train_count, user_id,
                                        k, test_count, recommendations, client_fd) == 0) {
            for (int j = 0; j < test_count; j++) {
                if (recommendations[j].item_id == item_id) {
                    float predicted_rating = recommendations[j].score;
                    float error = predicted_rating - actual_rating;
                    sum_squared_error += error * error;
                    sum_absolute_error += fabs(error);
                    valid_predictions++;
                    break;
                }
            }
        } else {
            char response[256];
            snprintf(response, sizeof(response), "Erreur pour user_id %d, item_id %d\n", user_id, item_id);
            write(client_fd, response, strlen(response));
        }
    }

    free(recommendations);

    if (valid_predictions == 0) {
        char response[256];
        snprintf(response, sizeof(response), "Aucune prédiction valide\n");
        write(client_fd, response, strlen(response));
        return -1;
    }

    *rmse = sqrt(sum_squared_error / valid_predictions);
    *mae = sum_absolute_error / valid_predictions;
    return 0;
}