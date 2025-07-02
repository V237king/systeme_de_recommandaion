#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include "../include/server.h"

int main() {
    ServerState state = {0};

    if (init_server_state("data/train_ratings.txt", "data/test_ratings.txt", &state) != 0) {
        printf("Erreur lors de l'initialisation du serveur\n");
        return 1;
    }

    printf("Serveur démarré, en écoute sur le port 8080...\n");
    if (start_server(8080, &state) != 0) {
        printf("Erreur lors du démarrage du serveur\n");
        free_server_state(&state);
        return 1;
    }

    free_server_state(&state); // Cette ligne ne sera atteinte que si start_server se termine (par Ctrl+C)
    return 0;
}