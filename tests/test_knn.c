#include <stdio.h>
#include <stdlib.h>
#include "./../include/data_reader.h"
#include "./../include/knn.h"

int main() {
    Transaction* train_transactions = NULL;
    int train_count = 0;
    Transaction* test_transactions = NULL;
    int test_count = 0;

    // Lire les données d'entraînement
    printf("Lecture des données d'entraînement :\n");
    if (read_ratings_file("data/train_ratings.txt", &train_transactions, &train_count,
                          0, 0, 0, 0, 0, 0, NULL, 0) != 0) {
        printf("Erreur lors de la lecture des données d'entraînement\n");
        return 1;
    }
    printf("Lu %d transactions d'entraînement :\n", train_count);
    for (int i = 0; i < train_count; i++) {
        printf("Transaction %d: user_id=%d, item_id=%d, category_id=%d, rating=%.1f, timestamp=%ld\n",
               i, train_transactions[i].user_id, train_transactions[i].item_id,
               train_transactions[i].category_id, train_transactions[i].rating,
               train_transactions[i].timestamp);
    }

    // Vérifier si user_id=123 existe
    int user_id = 123;
    int user_exists = 0;
    for (int i = 0; i < train_count; i++) {
        if (train_transactions[i].user_id == user_id) {
            user_exists = 1;
            break;
        }
    }
    printf("Vérification : user_id=%d %s dans les données d'entraînement\n",
           user_id, user_exists ? "existe" : "n'existe pas");

    // Lire les données de test
    printf("\nLecture des données de test avec nettoyage :\n");
    if (read_ratings_file("data/test_ratings.txt", &test_transactions, &test_count,
                          0, 0, 0, 0, 0, 1, train_transactions, train_count) != 0) {
        printf("Erreur lors de la lecture des données de test\n");
        free(train_transactions);
        return 1;
    }
    printf("Lu %d transactions de test nettoyées :\n", test_count);
    for (int i = 0; i < test_count; i++) {
        printf("Transaction %d: user_id=%d, item_id=%d, category_id=%d, rating=%.1f, timestamp=%ld\n",
               i, test_transactions[i].user_id, test_transactions[i].item_id,
               test_transactions[i].category_id, test_transactions[i].rating,
               test_transactions[i].timestamp);
    }

    // Tester les recommandations
    int k = 5;
    int num_recommendations = 3;
    Recommendation recommendations[3];
    printf("\nRecommandations pour user_id=%d (k=%d, num_recommendations=%d) :\n", user_id, k, num_recommendations);
    if (compute_knn_recommendations(train_transactions, train_count, user_id,
                                    k, num_recommendations, recommendations) == 0) {
        for (int i = 0; i < num_recommendations; i++) {
            printf("Recommandation %d: item_id=%d, score=%.2f\n",
                   i, recommendations[i].item_id, recommendations[i].score);
        }
    } else {
        printf("Erreur lors du calcul des recommandations\n");
    }

    // Tester les métriques RMSE et MAE
    float rmse, mae;
    if (compute_knn_metrics(train_transactions, train_count, test_transactions, test_count,
                            k, &rmse, &mae) == 0) {
        printf("\nMétriques KNN (k=%d) :\n", k);
        printf("RMSE = %.2f\n", rmse);
        printf("MAE = %.2f\n", mae);
    } else {
        printf("Erreur lors du calcul des métriques\n");
    }

    free(train_transactions);
    free(test_transactions);
    return 0;
}