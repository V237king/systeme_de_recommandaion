#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include "./../include/server.h"

int init_server_state(const char* train_file, const char* test_file, ServerState* state) {
    if (read_ratings_file(train_file, &state->train_transactions, &state->train_count,
                         0, 0, 0, 0, 0, 0, NULL, 0) != 0) {
        fprintf(stderr, "Erreur lors de la lecture des données d'entraînement\n");
        return -1;
    }

    if (read_ratings_file(test_file, &state->test_transactions, &state->test_count,
                         0, 0, 0, 0, 0, 1, state->train_transactions, state->train_count) != 0) {
        fprintf(stderr, "Erreur lors de la lecture des données de test\n");
        free(state->train_transactions);
        return -1;
    }

    int max_user_id = 0, max_item_id = 0;
    for (int i = 0; i < state->train_count; i++) {
        if (state->train_transactions[i].user_id > max_user_id) max_user_id = state->train_transactions[i].user_id;
        if (state->train_transactions[i].item_id > max_item_id) max_item_id = state->train_transactions[i].item_id;
    }
    state->mf_model = init_matrix_factorisation(max_user_id, max_item_id, 10);
    if (state->mf_model == NULL) {
        fprintf(stderr, "Erreur lors de l'initialisation du modèle MF\n");
        free(state->train_transactions);
        free(state->test_transactions);
        return -1;
    }

    if (train_matrix_factorisation(state->mf_model, state->train_transactions, state->train_count,
                                  0.01, 0.02, 100) != 0) {
        fprintf(stderr, "Erreur lors de l'entraînement du modèle MF\n");
        free_matrix_factorisation(state->mf_model);
        free(state->train_transactions);
        free(state->test_transactions);
        return -1;
    }

    state->page_rank_model = init_page_rank_model(state->train_transactions, state->train_count);
    if (state->page_rank_model == NULL) {
        fprintf(stderr, "Erreur lors de l'initialisation du modèle PageRank\n");
        free_matrix_factorisation(state->mf_model);
        free(state->train_transactions);
        free(state->test_transactions);
        return -1;
    }

    return 0;
}

void free_server_state(ServerState* state) {
    free(state->train_transactions);
    free(state->test_transactions);
    free_matrix_factorisation(state->mf_model);
    free_page_rank_model(state->page_rank_model);
}

int start_server(int port, ServerState* state) {
    int server_fd, client_fd;
    struct sockaddr_in server_addr, client_addr;
    socklen_t client_len = sizeof(client_addr);

    server_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (server_fd < 0) {
        perror("Erreur lors de la création du socket");
        return -1;
    }

    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = INADDR_ANY;
    server_addr.sin_port = htons(port);

    if (bind(server_fd, (struct sockaddr*)&server_addr, sizeof(server_addr)) < 0) {
        perror("Erreur lors du bind");
        close(server_fd);
        return -1;
    }

    if (listen(server_fd, 5) < 0) {
        perror("Erreur lors du listen");
        close(server_fd);
        return -1;
    }

    printf("Serveur en écoute sur le port %d...\n", port);

    while (1) {
        client_fd = accept(server_fd, (struct sockaddr*)&client_addr, &client_len);
        if (client_fd < 0) {
            perror("Erreur lors de l'accept");
            continue;
        }

        char buffer[256] = {0};
        read(client_fd, buffer, sizeof(buffer));
        char method[16];
        int user_id, k, num_recommendations;
        if (sscanf(buffer, "%s %d %d %d", method, &user_id, &k, &num_recommendations) != 4) {
            char* error = "Requête invalide\n";
            write(client_fd, error, strlen(error));
            close(client_fd);
            continue;
        }

        char response[1024] = {0};
        Recommendation recommendations[3];
        if (strcmp(method, "KNN") == 0) {
            if (compute_knn_recommendations(state->train_transactions, state->train_count,
                                           user_id, k, num_recommendations, recommendations,client_fd) == 0) {
                snprintf(response, sizeof(response), "item_id=%d:score=%.2f,item_id=%d:score=%.2f,item_id=%d:score=%.2f",
                         recommendations[0].item_id, recommendations[0].score,
                         recommendations[1].item_id, recommendations[1].score,
                         recommendations[2].item_id, recommendations[2].score);
            } else {
                response[0] = '\0';
            }
        } else if (strcmp(method, "MF") == 0) {
            if (compute_mf_recommendations(state->mf_model, state->train_transactions, state->train_count,
                                          user_id, num_recommendations, recommendations) == 0) {
                snprintf(response, sizeof(response), "item_id=%d:score=%.2f,item_id=%d:score=%.2f,item_id=%d:score=%.2f",
                         recommendations[0].item_id, recommendations[0].score,
                         recommendations[1].item_id, recommendations[1].score,
                         recommendations[2].item_id, recommendations[2].score);
            } else {
                snprintf(response, sizeof(response), "Erreur lors du calcul MF\n");
            }
        } else if (strcmp(method, "PAGERANK") == 0) {
            if (compute_page_rank_recommendations(state->page_rank_model, state->train_transactions, state->train_count,
                                                 user_id, num_recommendations, recommendations) == 0) {
                snprintf(response, sizeof(response), "item_id=%d:score=%.2f,item_id=%d:score=%.2f,item_id=%d:score=%.2f",
                         recommendations[0].item_id, recommendations[0].score,
                         recommendations[1].item_id, recommendations[1].score,
                         recommendations[2].item_id, recommendations[2].score);
            } else {
                snprintf(response, sizeof(response), "Erreur lors du calcul PageRank\n");
            }
        } else {
            snprintf(response, sizeof(response), "Méthode inconnue\n");
        }

        write(client_fd, response, strlen(response));
        close(client_fd);
    }

    close(server_fd);
    return 0;
}