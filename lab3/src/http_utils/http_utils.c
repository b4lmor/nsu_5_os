#include "../../include/http_utils.h"
#include "../../include/common.h"
#include "../../include/log.h"
#include <string.h>
#include <curl/curl.h>

typedef enum http_method {
    UNDEFINED = -1,
    GET,
    POST,
    PUT,
    DELETE,
    HEAD,
    PATCH
} __http_method_t;

int __parse_http_request(const char *request_bytes, http_request_t *request);

int __prepare_headers(struct curl_slist **headers, request_context_t *context);

__http_method_t __parse_method(const char *str);

static size_t callback(const void *contents, const size_t size, const size_t nmemb, void *userp) {
    const request_context_t *context = userp;
    add_chunk(context->manager, contents, size, nmemb, context->proxy_context);
    return size * nmemb;
}

static int xferinfo_callback(void *clientp, curl_off_t const dltotal, const curl_off_t dlnow, curl_off_t ultotal, curl_off_t ulnow) {
    request_context_t *context = clientp;
    if (dltotal > 0) {
        const double progress = dlnow / (double)dltotal * 100;
        if (progress >= 0 && context->downloaded_quarter == 0) {
            logurls(context->request->path, "Downloaded: 0% ... Started!");
            context->downloaded_quarter = 1;
        } else if (progress >= 33 && context->downloaded_quarter < 2) {
            logurls(context->request->path, "Downloaded: 33% ...");
            context->downloaded_quarter = 2;
        } else if (progress >= 66 && context->downloaded_quarter < 3) {
            logurls(context->request->path, "Downloaded: 66% ...");
            context->downloaded_quarter = 3;
        } else if (progress >= 100 && context->downloaded_quarter < 4) {
            logurls(context->request->path, "Downloaded: 100% ... Completed!");
            context->downloaded_quarter = 4;
        }
    }
    return 0;
}

int send_http_request(request_context_t *context) {
    CURL *curl = curl_easy_init();
    if (!curl) {
        perror("Failed to initialize libcurl");
        return -1;
    }
    long response = -1;

    curl_easy_setopt(curl, CURLOPT_URL, context->request->path);

    struct curl_slist *headers = NULL;
    if (__prepare_headers(&headers, context)) {
        perror("Failed to prepare headers");
        goto end;
    }
    curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);

    switch (__parse_method(context->request->method)) {
        case POST:
            curl_easy_setopt(curl, CURLOPT_POST, 1L);
            curl_easy_setopt(curl, CURLOPT_POSTFIELDS, context->request->body);
            curl_easy_setopt(curl, CURLOPT_POSTFIELDSIZE, strlen(context->request->body));
            break;
        case PUT:
            curl_easy_setopt(curl, CURLOPT_CUSTOMREQUEST, "PUT");
            curl_easy_setopt(curl, CURLOPT_POSTFIELDS, context->request->body);
            curl_easy_setopt(curl, CURLOPT_POSTFIELDSIZE, strlen(context->request->body));
            break;
        case DELETE:
            curl_easy_setopt(curl, CURLOPT_CUSTOMREQUEST, "DELETE");
            break;
        default:
            break;
    }

    curl_easy_setopt(curl, CURLOPT_ACCEPTTIMEOUT_MS, ACCEPT_TIMEOUT_MS);
    curl_easy_setopt(curl, CURLOPT_HEADERFUNCTION, callback);
    curl_easy_setopt(curl, CURLOPT_HEADERDATA, context);
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, callback);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, context);
    curl_easy_setopt(curl, CURLOPT_XFERINFOFUNCTION, xferinfo_callback);
    curl_easy_setopt(curl, CURLOPT_XFERINFODATA, context);
    curl_easy_setopt(curl, CURLOPT_NOPROGRESS, 0L);

    const CURLcode res = curl_easy_perform(curl);
    if (res != CURLE_OK) {
        fprintf(stderr, "curl_easy_perform() failed: %s\n", curl_easy_strerror(res));
    } else {
        curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &response);
    }
    curl_slist_free_all(headers);
end:
    curl_easy_cleanup(curl);
    return response;
}

request_context_t *create_request_context(http_request_t *request, subscription_manager_t *manager, proxy_context_t *proxy_context) {
    request_context_t *context = malloc(sizeof(request_context_t));
    if (context == NULL) {
        perror("Failed to allocate memory for request_context_t");
        return NULL;
    }
    context->manager = manager;
    context->request = request;
    context->proxy_context = proxy_context;
    context->downloaded_quarter = 0;
    return context;
}

http_request_t *parse_http_request(const char *request_bytes) {
    http_request_t *result = malloc(sizeof(http_request_t));
    if (!result) {
        perror("Failed to allocate memory for request");
        return NULL;
    }
    if (__parse_http_request(request_bytes, result)) {
        perror("Failed to parse http request");
        free(result);
        return NULL;
    }
    return result;
}

int __prepare_headers(struct curl_slist **headers, request_context_t *context) {
    *headers = curl_slist_append(*headers, "Accept-Encoding:");
    if (*headers == NULL) {
        perror("Failed to create curl slist");
        return -1;
    }
    for (int i = 0; i < context->request->header_count; i++) {
        *headers = curl_slist_append(*headers, context->request->headers[i]);
    }
    return 0;
}

int __parse_http_request(const char *request_bytes, http_request_t *request) {
    const char *ptr = request_bytes;
    const char *line_end = strstr(ptr, "\r\n");
    if (line_end == NULL) {
        line_end = strstr(ptr, "\n");
    }
    if (line_end) {
        sscanf(ptr, "%7s %255s %15s", request->method, request->path, request->version);
        ptr = line_end + 2;
    }
    request->header_count = 0;
    while (
        (line_end = strstr(ptr, "\r\n")) != NULL
        || (line_end = strstr(ptr, "\n")) != NULL
    ) {
        if (line_end == NULL) break;
        size_t header_length = line_end - ptr;
        if (header_length > 255) {
            header_length = 255;
        }
        strncpy(request->headers[request->header_count], ptr, header_length);
        request->headers[request->header_count][header_length] = '\0';
        request->header_count++;
        if (request->header_count >= 10) break;
        ptr = line_end + 2;
    }
    if (strlen(ptr) > 0) {
        strncpy(request->body, ptr, sizeof(request->body) - 1);
        request->body[sizeof(request->body) - 1] = '\0';
    } else {
        request->body[0] = '\0';
    }
    return 0;
}

__http_method_t __parse_method(const char *str) {
    __http_method_t method = UNDEFINED;
    if (strcmp(str, "GET") == 0) {
        method = GET;
    } else if (strcmp(str, "POST") == 0) {
        method = POST;
    } else if (strcmp(str, "PUT") == 0) {
        method = PUT;
    } else if (strcmp(str, "DELETE") == 0) {
        method = DELETE;
    } else if (strcmp(str, "HEAD") == 0) {
        method = HEAD;
    } else if (strcmp(str, "PATCH") == 0) {
        method = PATCH;
    } else {
        logsss("__parse_method()", "Unsupported method", str);
    }
    return method;
}
