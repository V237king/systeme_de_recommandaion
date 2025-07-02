#include <stdio.h>
#include <stdlib.h>
#include "./../include/data_reader.h"

int main() {
    Transaction* train_transactions = NULL; // Tableau dynamique pour l'entraînement
    int train_count = 0;
    Transaction* test_transactions = NULL; // Tableau dynamique pour le test
    int test_count = 0;

    // Lire les données d'entraînement (sans filtrage)
    printf("Lecture des données d'entraînement :\n");
    if (read_ratings_file("data/train_ratings.txt", &train_transactions, &train_count,
                          0, 0, 0, 0, 0, 0, NULL, 0) == 0) {
        printf("Lu %d transactions d'entraînement :\n", train_count);
        for (int i = 0; i < train_count; i++) {
            printf("Transaction %d: user_id=%d, item_id=%d, category_id=%d, rating=%.1f, timestamp=%ld\n",
                   i, train_transactions[i].user_id, train_transactions[i].item_id,
                   train_transactions[i].category_id, train_transactions[i].rating,
                   train_transactions[i].timestamp);
        }
    } else {
        printf("Erreur lors de la lecture des données d'entraînement\n");
        free(train_transactions);
        return 1;
    }

    // Lire les données de test avec nettoyage
    printf("\nLecture des données de test avec nettoyage :\n");
    if (read_ratings_file("data/test_ratings.txt", &test_transactions, &test_count,
                          0, 0, 0, 0, 0, 1, train_transactions, train_count) == 0) {
        printf("Lu %d transactions de test nettoyées :\n", test_count);
        for (int i = 0; i < test_count; i++) {
            printf("Transaction %d: user_id=%d, item_id=%d, category_id=%d, rating=%.1f, timestamp=%ld\n",
                   i, test_transactions[i].user_id, test_transactions[i].item_id,
                   test_transactions[i].category_id, test_transactions[i].rating,
                   test_transactions[i].timestamp);
        }
    } else {
        printf("Erreur lors de la lecture des données de test\n");
        free(train_transactions);
        free(test_transactions);
        return 1;
    }

    // Libérer la mémoire
    free(train_transactions);
    free(test_transactions);
    return 0;
}