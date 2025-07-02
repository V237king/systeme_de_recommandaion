#include <stdio.h>
#include <stdlib.h>
#include "./../include/data_reader.h"
#include "./../include/page_rank_recommendation.h"

int main() {
    Transaction* train_transactions = NULL;
    int train_count = 0;

    printf("Lecture des données d'entraînement :\n");
    if (read_ratings_file("data/train_ratings.txt", &train_transactions, &train_count,
                          0, 0, 0, 0, 0, 0, NULL, 0) != 0) {
        printf("Erreur lors de la lecture des données d'entraînement\n");
        return 1;
    }
    printf("Lu %d transactions d'entraînement :\n", train_count);
    for (int i = 0; i < train_count && i < 10; i++) {
        printf("Transaction %d: user_id=%d, item_id=%d, category_id=%d, rating=%.1f, timestamp=%ld\n",
               i, train_transactions[i].user_id, train_transactions[i].item_id,
               train_transactions[i].category_id, train_transactions[i].rating,
               train_transactions[i].timestamp);
    }

    PageRankModel* model = init_page_rank_model(train_transactions, train_count);
    if (model == NULL) {
        printf("Erreur lors de l'initialisation du modèle\n");
        free(train_transactions);
        return 1;
    }

    int user_id = 1;
    int num_recommendations = 3;
    Recommendation recommendations[3];
    printf("\nRecommandations pour user_id=%d (num_recommendations=%d) avec PageRank :\n", user_id, num_recommendations);
    if (compute_page_rank_recommendations(model, train_transactions, train_count, user_id,
                                          num_recommendations, recommendations) == 0) {
        for (int i = 0; i < num_recommendations; i++) {
            printf("Recommandation %d: item_id=%d, score=%.2f\n",
                   i, recommendations[i].item_id, recommendations[i].score);
        }
    } else {
        printf("Erreur lors du calcul des recommandations\n");
    }

    free_page_rank_model(model);
    free(train_transactions);
    return 0;
}