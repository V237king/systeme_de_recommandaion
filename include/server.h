#ifndef SERVER_H
#define SERVER_H

#include "data_reader.h"
#include "knn.h"
#include "matrix_factorisation.h"
#include "page_rank_recommendation.h"

// Structure pour stocker l'état du serveur
typedef struct {
    Transaction* train_transactions;
    int train_count;
    Transaction* test_transactions;
    int test_count;
    MatrixFactorisationModel* mf_model;
    PageRankModel* page_rank_model;
} ServerState;

// Déclaration des fonctions
int start_server(int port, ServerState* state);
int init_server_state(const char* train_file, const char* test_file, ServerState* state);
void free_server_state(ServerState* state);

#endif