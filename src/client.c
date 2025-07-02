#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <signal.h>

int main(int argc, char* argv[]) {
    if (argc != 5) {
        printf("Usage: %s <host> <port> <method> <user_id>\n", argv[0]);
        return 1;
    }

    const char* host = argv[1];
    int port = atoi(argv[2]);
    const char* method = argv[3];
    int user_id = atoi(argv[4]);
    int k = 5; // Par défaut pour KNN
    int num_recommendations = 3;

    int sock = socket(AF_INET, SOCK_STREAM, 0);
    if (sock < 0) {
        perror("Erreur lors de la création du socket");
        return 1;
    }

    struct sockaddr_in server_addr;
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(port);
    inet_pton(AF_INET, host, &server_addr.sin_addr);

    if (connect(sock, (struct sockaddr*)&server_addr, sizeof(server_addr)) < 0) {
        perror("Erreur lors de la connexion");
        close(sock);
        return 1;
    }

    char request[256];
    snprintf(request, sizeof(request), "%s %d %d %d", method, user_id, k, num_recommendations);
    write(sock, request, strlen(request));

    char response[1024] = {0};
    read(sock, response, sizeof(response));
    printf("Réponse du serveur : %s\n", response);

    close(sock);
    return 0;
}