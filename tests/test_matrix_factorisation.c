#include <stdio.h>
#include <stdlib.h>
#include "./../include/data_reader.h"
#include "./../include/matrix_factorisation.h"

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
    for (int i = 0; i < train_count && i < 10; i++) { // Limiter l'affichage
        printf("Transaction %d: user_id=%d, item_id=%d, category_id=%d, rating=%.1f, timestamp=%ld\n",
               i, train_transactions[i].user_id, train_transactions[i].item_id,
               train_transactions[i].category_id, train_transactions[i].rating,
               train_transactions[i].timestamp);
    }

    // Lire les données de test
    printf("\nLecture des données de test avec nettoyage :\n");
    if (read_ratings_file("data/test_ratings.txt", &test_transactions, &test_count,
                          0, 0, 0, 0, 0, 1, train_transactions, train_count) != 0) {
        printf("Erreur lors de la lecture des données de test\n");
        free(train_transactions);
        return 1;
    }
    printf("Lu %d transactions de test nettoyées :\n", test_count);
    for (int i = 0; i < test_count && i < 10; i++) { // Limiter l'affichage
        printf("Transaction %d: user_id=%d, item_id=%d, category_id=%d, rating=%.1f, timestamp=%ld\n",
               i, test_transactions[i].user_id, test_transactions[i].item_id,
               test_transactions[i].category_id, test_transactions[i].rating,
               test_transactions[i].timestamp);
    }

    // Initialiser et entraîner le modèle
    int num_factors = 10;
    int max_user_id = 0, max_item_id = 0;
    for (int i = 0; i < train_count; i++) {
        if (train_transactions[i].user_id > max_user_id) max_user_id = train_transactions[i].user_id;
        if (train_transactions[i].item_id > max_item_id) max_item_id = train_transactions[i].item_id;
    }

    MatrixFactorisationModel* model = init_matrix_factorisation(max_user_id, max_item_id, num_factors);
    if (model == NULL) {
        printf("Erreur lors de l'initialisation du modèle\n");
        free(train_transactions);
        free(test_transactions);
        return 1;
    }

    printf("\nEntraînement du modèle de factorisation de matrices...\n");
    if (train_matrix_factorisation(model, train_transactions, train_count, 0.01, 0.02, 100) != 0) {
        printf("Erreur lors de l'entraînement\n");
        free_matrix_factorisation(model);
        free(train_transactions);
        free(test_transactions);
        return 1;
    }

    // Tester les recommandations pour un utilisateur
    int user_id = 1; // Changer si nécessaire
    int num_recommendations = 3;
    Recommendation recommendations[3];
    printf("\nRecommandations pour user_id=%d (num_recommendations=%d) :\n", user_id, num_recommendations);
    if (compute_mf_recommendations(model, train_transactions, train_count, user_id,
                                   num_recommendations, recommendations) == 0) {
        for (int i = 0; i < num_recommendations; i++) {
            printf("Recommandation %d: item_id=%d, score=%.2f\n",
                   i, recommendations[i].item_id, recommendations[i].score);
        }
    } else {
        printf("Erreur lors du calcul des recommandations\n");
    }

    // Tester les métriques RMSE et MAE
    float rmse, mae;
    if (compute_mf_metrics(model, test_transactions, test_count, &rmse, &mae) == 0) {
        printf("\nMétriques Matrix Factorisation :\n");
        printf("RMSE = %.2f\n", rmse);
        printf("MAE = %.2f\n", mae);
    } else {
        printf("Erreur lors du calcul des métriques\n");
    }

    // Libérer la mémoire
    free_matrix_factorisation(model);
    free(train_transactions);
    free(test_transactions);
    return 0;
}