#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>

#include "../../include/server.h"
#include "../../include/handler.h"

int start_server(const int port, proxy_context_t *context) {
    struct sockaddr_in server_addr, client_addr;
    socklen_t client_len = sizeof(client_addr);

    const int server_socket = socket(AF_INET, SOCK_STREAM, 0);
    if (server_socket < 0) {
        perror("socket");
        return -1;
    }

    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = INADDR_ANY;
    server_addr.sin_port = htons(port);

    if (bind(server_socket, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0) {
        perror("bind");
        close(server_socket);
        return EXIT_FAILURE;
    }

    listen(server_socket, CONNECTIONS_NUMBER);

    char server_ip[INET_ADDRSTRLEN];
    inet_ntop(AF_INET, &server_addr.sin_addr, server_ip, sizeof(server_ip));
    printf("[SERVER] :: Listening on socket %s:%d ...\n", server_ip, port);

    while (1) {
        const int client_socket = accept(server_socket, (struct sockaddr *) &client_addr, &client_len);
        if (client_socket < 0) {
            perror("accept");
            continue;
        }

        char client_ip[INET_ADDRSTRLEN];
        inet_ntop(AF_INET, &client_addr.sin_addr, client_ip, sizeof(client_ip));
        const int client_port = ntohs(client_addr.sin_port);
        printf("[SERVER] :: Connection established with %s:%d\n", client_ip, client_port);

        handle_client(client_socket, context);
    }

    close(server_socket);
    return 0;
}
