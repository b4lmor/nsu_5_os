 #include "../../include/http_utils.h"
#include "../../include/common.h"
#include <string.h>
#include <curl/curl.h>

static size_t callback(const void *contents, const size_t size, const size_t nmemb, FILE *userp) {
    return fwrite(contents, size, nmemb, userp);
}

static int xferinfo_callback(void *clientp, curl_off_t const dltotal, const curl_off_t dlnow, curl_off_t ultotal, curl_off_t ulnow) {
    HttpContext *context = clientp;
    if (dltotal > 0) {
        const double progress = dlnow / (double)dltotal * 100;
        if (progress >= 33 && context->last_logged_percentage < 33) {
            printf("[%d] :: Downloaded: %lld / %lld bytes (33%%)\n", context->id, dlnow, dltotal);
            context->last_logged_percentage = 33;
        } else if (progress >= 66 && context->last_logged_percentage < 66) {
            printf("[%d] :: Downloaded: %lld / %lld bytes (66%%)\n", context->id, dlnow, dltotal);
            context->last_logged_percentage = 66;
        } else if (progress >= 100 && context->last_logged_percentage < 100) {
            printf("[%d] :: Downloaded: %lld / %lld bytes (100%%)\n", context->id, dlnow, dltotal);
            context->last_logged_percentage = 100;
        }
    }
    return 0;
}

static int parse_method(const char *str) {
    Methods method = -1;
    if (strcmp(str, "CONNECT") == 0) {
        method = CONNECT;
    } else if (strcmp(str, "GET") == 0) {
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
        perror("Can't parse method");
    }
    return method;
}

int send_http_request(const HttpRequest *request, HttpContext *context) {
    CURL *curl = curl_easy_init();
    if (!curl) {
        fprintf(stderr, "Failed to initialize libcurl\n");
        return -1;
    }

    char url[URL_MAX_SIZE];
    snprintf(url, sizeof(url), "%s", request->path);
    curl_easy_setopt(curl, CURLOPT_URL, url);

    struct curl_slist *headers = NULL;
    headers = curl_slist_append(headers, "Accept-Encoding:");
    for (int i = 0; i < request->header_count; i++) {
        headers = curl_slist_append(headers, request->headers[i]);
    }
    curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);

    switch (parse_method(request->method)) {
        case GET:
            curl_easy_setopt(curl, CURLOPT_HTTPGET, 1L);
            break;
        case POST:
            curl_easy_setopt(curl, CURLOPT_POST, 1L);
            curl_easy_setopt(curl, CURLOPT_POSTFIELDS, request->body);
            curl_easy_setopt(curl, CURLOPT_POSTFIELDSIZE, strlen(request->body));
            break;
        case PUT:
            curl_easy_setopt(curl, CURLOPT_CUSTOMREQUEST, "PUT");
            curl_easy_setopt(curl, CURLOPT_POSTFIELDS, request->body);
            curl_easy_setopt(curl, CURLOPT_POSTFIELDSIZE, strlen(request->body));
            break;
        case DELETE:
            curl_easy_setopt(curl, CURLOPT_CUSTOMREQUEST, "DELETE");
            break;
        default:
            break;
    }

    char tmp_filename[20];
    snprintf(tmp_filename, 20, "../cache/%d.tmp", context->id);
    FILE *tmp_file = fopen(tmp_filename, "wb");
    if (!tmp_file) {
        fprintf(stderr, "Failed to open temporary file\n");
        curl_slist_free_all(headers);
        curl_easy_cleanup(curl);
        return -1;
    }

    curl_easy_setopt(curl, CURLOPT_ACCEPTTIMEOUT_MS, 5000L);
    curl_easy_setopt(curl, CURLOPT_HEADERFUNCTION, callback);
    curl_easy_setopt(curl, CURLOPT_HEADERDATA, tmp_file);
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, callback);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, tmp_file);
    curl_easy_setopt(curl, CURLOPT_XFERINFOFUNCTION, xferinfo_callback);
    curl_easy_setopt(curl, CURLOPT_XFERINFODATA, context);
    curl_easy_setopt(curl, CURLOPT_NOPROGRESS, 0L);

    const CURLcode res = curl_easy_perform(curl);
    if (res != CURLE_OK) {
        fprintf(stderr, "curl_easy_perform() failed: %s\n", curl_easy_strerror(res));
        curl_slist_free_all(headers);
        curl_easy_cleanup(curl);
        return -1;
    }

    fclose(tmp_file);

    time_t now;
    time(&now);
    struct tm *local = localtime(&now);
    fprintf(context->out, TIME_FORMAT, local->tm_mday, local->tm_mon + 1,
        local->tm_year + 1900, local->tm_hour, local->tm_min, local->tm_sec);

    tmp_file = fopen(tmp_filename, "rb");
    append_file(tmp_file, context->out);
    fclose(tmp_file);
    remove(tmp_filename);

    curl_slist_free_all(headers);
    curl_easy_cleanup(curl);
    return 0;
}

int parse_http_request(const char *raw_request, HttpRequest *request) {
    const char *ptr = raw_request;
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
