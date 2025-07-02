#ifndef KNN_H
#define KNN_H

#include "data_reader.h"

// Calcule les recommandations KNN pour un utilisateur
// Paramètres : transactions (tableau de transactions), count (nombre de transactions),
//              user_id (utilisateur cible), k (nombre de voisins), num_recommendations (nombre de recommandations),
//              recommendations (tableau de sortie)
// Retour : 0 si succès, -1 si erreur
int compute_knn_recommendations(Transaction* transactions, int count, int user_id,
                                int k, int num_recommendations, Recommendation* recommendations, int client_fd);

// Calcule les métriques RMSE et MAE pour KNN
// Paramètres : train_transactions (données d'entraînement), train_count (nombre de transactions d'entraînement),
//              test_transactions (données de test), test_count (nombre de transactions de test),
//              k (nombre de voisins), rmse (pointeur pour RMSE), mae (pointeur pour MAE)
// Retour : 0 si succès, -1 si erreur
 float compute_pearson_correlation(Transaction* transactions, int count,
                                 int user1_id, int user2_id, int client_fd);
int compute_knn_metrics(Transaction* train_transactions, int train_count,
                        Transaction* test_transactions, int test_count,
                        int k, float* rmse, float* mae, int client_fd);

#endif