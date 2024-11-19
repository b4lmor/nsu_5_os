#ifndef HTTP_UTILS_H
#define HTTP_UTILS_H

#include <stdio.h>

#define LOG_ITERATION_COUNT 30000
#define MAX_REQUEST_SIZE 4096
#define URL_MAX_SIZE 512
#define SUPPORTED_VERSION "HTTP/1.0"
#define TIME_FORMAT "%02d-%02d-%04d %02d:%02d:%02d\n"

typedef struct {
    int id;
    FILE *out;
    size_t last_logged_percentage;
} HttpContext;

typedef struct {
    int header_count;
    char method[8];
    char version[16];
    char path[URL_MAX_SIZE];
    char body[512];
    char headers[10][256];
} HttpRequest;

typedef enum {
    UNDEFINED = -1,
    CONNECT = 100,
    GET = 1,
    POST = 2,
    PUT = 3,
    DELETE = 4,
    HEAD = 5,
    PATCH = 6,
} Methods;

int parse_http_request(const char *raw_request, HttpRequest *request);

int send_http_request(const HttpRequest *request, HttpContext *context);

#endif
