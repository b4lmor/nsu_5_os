#include <string.h>
#include <sys/socket.h>

#include "../../include/handler.h"
#include "../../include/http_utils.h"
#include "../../include/log.h"

typedef struct client_data {
    proxy_context_t *context;
    int fd;
} client_data_t;

void *__handle_client(void *arg);

int __recv(int fd, char *buffer);

int __validate_request(const http_request_t *request);

int __check_cache(const http_request_t *request);

int __check_response_code(int code);

int handle_client(const int client_socket, proxy_context_t *context) {
    client_data_t *data = malloc(sizeof(client_data_t));
    data->context = context;
    data->fd = client_socket;
    __handle_client(data); // todo: make parallel
    return 0;
}

// @parallel
void *__handle_client(void *arg) {
    client_data_t *data = arg;
    const int fd = data->fd;

    char request_buf[HTTP_REQUEST_BYTES_LEN] = {0};
    if (__recv(fd, request_buf) < 0) {
        goto end1;
    }

    http_request_t *request = parse_http_request(request_buf);
    if (request == NULL) {
        perror("parse_http_request");
        goto end1;
    }

    logi(fd, request->path);

    if (__validate_request(request) < 0) {
        logi(fd, "invalid request!");
        goto end2;
    }

    subscription_manager_t *manager = get(data->context->map, request->path);
    if (manager == NULL) {
        manager = create_subscription_manager();
        if (manager == NULL) {
            perror("create_subscription_manager");
            goto end2;
        }
        insert(data->context->map, request->path, manager);
    }

    subscribe(manager, fd); // manager ==> current

    if (__check_cache(request) < 0) {
        logi(fd, "cache file not found.");
        if (cas_state(manager, 0, 1)) {
            request_context_t *request_context = create_request_context(request, manager);
            const int response_code = send_http_request(request_context); // data => manager ==> all subscribers
            if (__check_response_code(response_code) == 0) {
                // todo: save chunk container as cache
            }
        }
    } else {
        logi(fd, "cache file found!");
        // todo: implement cache logic // data => manager ==> all subscribers
    }
end2:
    free(request);
end1:
    free(arg);
    return NULL;
}

int __recv(const int fd, char *buffer) {
    const int bytes_read = recv(fd, buffer, HTTP_REQUEST_BYTES_LEN - 1, 0);
    if (bytes_read < 0) {
        perror("recv");
        return -1;
    }
    buffer[bytes_read] = '\0';
    return 0;
}

int __validate_request(const http_request_t *request) {
    if (request == NULL) {
        return -1;
    }
    if (strncmp(request->version, "HTTP/1.0", HTTP_VERSION_LEN) != 0 &&
        strncmp(request->version, "HTTP/1.1", HTTP_VERSION_LEN) != 0) {
        return -1;
    }
    return 0;
}

int __check_cache(const http_request_t *request) {
    return -1; // todo add cache
}

int __check_response_code(const int code) {
    if (code >= 200 && code < 300) {
        return 0;
    }
    return -1;
}
