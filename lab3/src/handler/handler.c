#include "../../include/handler.h"
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>
#include "../../include/http_utils.h"
#include "../../include/log.h"
#include "../../include/cache.h"
#include "../../include/common.h"

typedef struct client_data {
    proxy_context_t *context;
    int fd;
} client_data_t;

typedef struct session_data {
    subscription_manager_t *manager;
    http_request_t *request;
    proxy_context_t *context;
} session_data_t;

void *__handle_client(void *arg);

void *__share_cache(void *arg);

void *__download_and_send(void *arg);

session_data_t *__create_session_data(subscription_manager_t *manager, http_request_t *request, proxy_context_t *context);

int __recv(int fd, char *buffer);

int __validate_request(const http_request_t *request);

int __check_cache(const http_request_t *request);

int __check_response_code(int code);

int handle_client(const int client_socket, proxy_context_t *context) {
    client_data_t *data = malloc(sizeof(client_data_t));
    data->context = context;
    data->fd = client_socket;
#ifdef PARALLEL
    threadpool_push_task(context->handler_pool, __handle_client, data);
#else
    __handle_client(data);
#endif
    return 0;
}

// @parallel
void *__handle_client(void *arg) {
    client_data_t *data = arg;
    const int fd = data->fd;
    http_request_t *request = NULL;

    char request_buf[HTTP_REQUEST_BYTES_LEN] = {0};
    if (__recv(fd, request_buf) < 0) {
        close(fd);
        goto end;
    }

    request = parse_http_request(request_buf);
    if (request == NULL) {
        close(fd);
        goto end;
    }

    logiss(fd, "request url:", request->path);

    if (__validate_request(request) < 0) {
        logis(fd, "invalid request!");
        close(fd);
        goto end;
    }

    subscription_manager_t *manager = get(data->context->map, request->path);
    if (manager == NULL) {
        logis(fd, "manager not found");
        manager = create_subscription_manager();
        if (manager == NULL) {
            close(fd);
            goto end;
        }
        insert(data->context->map, request->path, manager);
    }

    subscribe(manager, fd);

    if (cas(&manager->is_busy, 0, 1)) { // true => this thread is session main thread
        logis(fd, "manager is not busy");
        session_data_t *session = __create_session_data(manager, request, data->context);
        if (session == NULL) {
            destroy_subscription_manager(&manager);
            delete(data->context->map, request->path);
            goto end;
        }
        if (__check_cache(request) != 0) {
            logis(fd, "cache file not found.");
            __download_and_send(session);
        } else {
            logis(fd, "cache file found!");
            __share_cache(session);
        }
        logis(fd, "request complete! finishing ...");
        finish_pending_chunks(manager);
        destroy_subscription_manager(&manager);
        delete(data->context->map, request->path);
        logis(fd, "request complete! finishing ... done");
    }
end:
    free(request);
    free(arg);
    return NULL;
}

void *__download_and_send(void *arg) {
    session_data_t *session = arg;
    request_context_t *request_context = create_request_context(session->request, session->manager, session->context);
    logss(session->request->path, "sending request...");
    const int response_code = send_http_request(request_context);
    logss(session->request->path, "sending request... done");
    if (__check_response_code(response_code) == 0) {
        char *cachename = parse_request_to_cachename(session->request);
        logss(session->request->path, "saving cache ...");
        save_cache(session->manager, cachename);
        logss(session->request->path, "saving cache ... done");
        free(cachename);
    }
    free(session);
    return NULL;
}

void * __share_cache(void *arg) {
    session_data_t *session = arg;
    char *cachename = parse_request_to_cachename(session->request);
    share_cache(session->manager, cachename, session->context);
    free(cachename);
    free(session);
    return NULL;
}

session_data_t * __create_session_data(subscription_manager_t *manager, http_request_t *request, proxy_context_t *context) {
    session_data_t *session = malloc(sizeof(session_data_t));
    if (session == NULL) {
        perror("malloc session context");
        return NULL;
    }
    session->request = request;
    session->manager = manager;
    session->context = context;
    return session;
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
    if (
        (strncmp(request->version, "HTTP/1.0", HTTP_VERSION_LEN) != 0 &&
         strncmp(request->version, "HTTP/1.1", HTTP_VERSION_LEN) != 0)
        || strncmp(request->path, "http://o.pki.goog/we2", HTTP_PATH_LEN) == 0
    ) {
        return -1;
    }
    return 0;
}

int __check_cache(const http_request_t *request) {
    char *cachename = parse_request_to_cachename(request);
    const int result = cache_exists(cachename);
    free(cachename);
    return result;
}

int __check_response_code(const int code) {
    if (code >= 200 && code < 300) {
        return 0;
    }
    return -1;
}
