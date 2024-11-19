#define _GNU_SOURCE
#include <stdio.h>
#include <pthread.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include "../../include/handler.h"
#include "../../include/http_utils.h"
#include "../../include/common.h"

#define PARALLEL 0

#define CACHE_TTL_MS 1000000
#define MAX_CACHE_NAME_LENGTH 30

typedef struct {
    Context *context;
    int fd;
} Data;

int receive_client_request(int client_socket, char *buffer);

void serve_cached_file(const char *cache_filename, int client_socket);

int fetch_and_cache_request(const HttpRequest *request, const char *cache_filename, int fd);

int validate_request(const HttpRequest *request);

void *__handle_client(void *arg) {
    const Data *data = arg;
    const int client_socket = data->fd;

    char buffer_request[MAX_REQUEST_SIZE] = {0};
    if (receive_client_request(client_socket, buffer_request) < 0) {
        goto finally;
    }

    HttpRequest request;
    if (parse_http_request(buffer_request, &request) < 0) {
        perror("parse_http_request");
        goto finally;
    }

    const int cache_hash = hash(request.path) % data->context->mutex_number;
    printf("[%d] :: Request url: %s | hash: %d\n", client_socket, request.path, cache_hash);

    if (strncmp(request.version, "HTTPS", 5) == 0) {
        printf("Got HTTPS request!\n");
        goto finally;
    }

    if (!validate_request(&request)) {
        printf("[%d] :: Warning: Expected protocol '%s', but got '%s'\n",
               client_socket, SUPPORTED_VERSION, request.version);
    }

    pthread_mutex_lock(&data->context->mutexes[cache_hash]);

    char *cache_file_name = calloc(MAX_CACHE_NAME_LENGTH, sizeof(char));
    snprintf(cache_file_name, MAX_CACHE_NAME_LENGTH - 1, "../cache/%d.cache", cache_hash);

    if ((data->context->last_cached_urls[cache_hash] == NULL
        ||strncmp(cache_file_name, data->context->last_cached_urls[cache_hash], MAX_CACHE_NAME_LENGTH) == 0)
        && check_cache_expiration(cache_file_name, CACHE_TTL_MS) == 0) {
        printf("[%d] :: Cache file found!\n", client_socket);
    } else {
        printf("[%d] :: Cache file not found. Sending request ...\n", client_socket);
        if (fetch_and_cache_request(&request, cache_file_name, client_socket) < 0) {
            printf("[%d] :: Failed to send request\n", client_socket);
            pthread_mutex_unlock(&data->context->mutexes[cache_hash]);
            goto finally;
        }
    }

    serve_cached_file(cache_file_name, client_socket);

    if (data->context->last_cached_urls[cache_hash] != NULL) {
        free(data->context->last_cached_urls[cache_hash]);
    }
    data->context->last_cached_urls[cache_hash] = cache_file_name;

    pthread_mutex_unlock(&data->context->mutexes[cache_hash]);

finally:
    close(client_socket);
    printf("[%d] :: Socket closed\n", client_socket);
    free(arg);
    return NULL;
}

int handle_client(const int client_socket, Context *context) {
    Data *data = malloc(sizeof(Data));
    data->context = context;
    data->fd = client_socket;
#if PARALLEL
    pthread_t thread;
    if (pthread_create(&thread, NULL, __handle_client, data) < 0) {
        perror("pthread_create");
        free(data);
        close(client_socket);
        printf("[%d] :: Socket closed\n", client_socket);
        return -1;
    }
#else
    __handle_client(data);
#endif
    return 0;
}

int receive_client_request(const int client_socket, char *buffer) {
    const int bytes_read = recv(client_socket, buffer, MAX_REQUEST_SIZE - 1, 0);
    if (bytes_read < 0) {
        perror("recv");
        return -1;
    }
    buffer[bytes_read] = '\0';
    return 0;
}

int fetch_and_cache_request(const HttpRequest *request, const char *cache_filename, const int fd) {
    FILE *cache_file = fopen(cache_filename, "ab");
    if (!cache_file) return -1;
    HttpContext context;
    context.id = fd;
    context.out = cache_file;
    context.last_logged_percentage = 0;
    const int err = send_http_request(request, &context);
    fclose(cache_file);
    return err;
}

void serve_cached_file(const char *cache_filename, const int client_socket) {
    FILE *output_file = fopen(cache_filename, "rb");
    if (output_file) {
        write_file_to_fd(output_file, client_socket, 1);
        if (ferror(output_file)) {
            perror("Error reading from file");
        }
        fclose(output_file);
    } else {
        perror("Error opening cache file");
    }
}

int validate_request(const HttpRequest *request) {
    return strncmp(request->version, SUPPORTED_VERSION, strlen(SUPPORTED_VERSION)) == 0;
}
