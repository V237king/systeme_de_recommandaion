#ifndef PAGE_RANK_RECOMMENDATION_H
#define PAGE_RANK_RECOMMENDATION_H

#include "data_reader.h"

// Structure pour stocker le modèle de recommandation basé sur PageRank
typedef struct {
    float** adjacency_matrix; // Matrice d'adjacence (utilisateurs × items)
    int num_users; // Nombre d'utilisateurs
    int num_items; // Nombre d'items
} PageRankModel;

// Initialise le modèle de recommandation basé sur PageRank
// Paramètres : transactions (données d'entraînement), count (nombre de transactions)
// Retour : pointeur vers le modèle, ou NULL si erreur
PageRankModel* init_page_rank_model(Transaction* transactions, int count);

// Calcule les recommandations pour un utilisateur avec PageRank
// Paramètres : model (modèle PageRank), transactions (données d'entraînement),
//              count (nombre de transactions), user_id (utilisateur cible),
//              num_recommendations (nombre de recommandations),
//              recommendations (tableau de sortie)
// Retour : 0 si succès, -1 si erreur
int compute_page_rank_recommendations(PageRankModel* model, Transaction* transactions,
                                      int count, int user_id, int num_recommendations,
                                      Recommendation* recommendations);

// Libère la mémoire du modèle
void free_page_rank_model(PageRankModel* model);

#endif