#ifndef HTTP_UTILS_H
#define HTTP_UTILS_H

#include <stdio.h>
#include "subscription.h"

#define HTTP_REQUEST_BYTES_LEN 4096

#define HTTP_METHOD_LEN 8
#define HTTP_VERSION_LEN 16
#define HTTP_PATH_LEN 512
#define HTTP_BODY_LEN 512
#define HTTP_HEADER_COUNT 16
#define HTTP_HEADER_LEN 256

#define ACCEPT_TIMEOUT_MS 5000L

typedef struct http_request {
    size_t header_count;
    char method[HTTP_METHOD_LEN];
    char version[HTTP_VERSION_LEN];
    char path[HTTP_PATH_LEN];
    char body[HTTP_BODY_LEN];
    char headers[HTTP_HEADER_COUNT][HTTP_HEADER_LEN];
} http_request_t;

typedef struct request_context {
    size_t downloaded;
    http_request_t *request;
    subscription_manager_t *manager;
} request_context_t;

request_context_t *create_request_context(http_request_t *request, subscription_manager_t *manager);

http_request_t* parse_http_request(const char *request_bytes);

int send_http_request(request_context_t *context);

#endif
