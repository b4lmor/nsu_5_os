#include "../../include/cache.h"

#include <stdio.h>
#include <string.h>
#include <unistd.h>

#include "../../include/common.h"
#include "../../include/log.h"
#include <sys/stat.h>

#define CACHENAME_LEN 512

void share_cache(subscription_manager_t *manager, const char *cachename, proxy_context_t *proxy_context) {

    logsss("CACHE", "cache file name:", cachename);

    char full_cachename[CACHENAME_LEN + 10];
    sprintf(full_cachename, "../cache/%s", cachename);
    FILE *file = fopen(full_cachename, "rb");
    if (!file) {
        perror("Unable to open file");
        return;
    }

    char buffer[CACHE_BUFFER_SIZE];
    size_t bytes_read;

    while ((bytes_read = fread(buffer, 1, CACHE_BUFFER_SIZE, file)) > 0) {
        add_chunk(manager, buffer, bytes_read, 1, proxy_context);
    }

    if (ferror(file)) {
        perror("Error reading file");
    }

    fclose(file);
}

void save_cache(const subscription_manager_t *manager, const char *cachename) {
    char full_cachename[CACHENAME_LEN + 10];
    sprintf(full_cachename, "../cache/%s", cachename);
    FILE *file = fopen(full_cachename, "wb");
    if (!file) {
        perror("Unable to open file");
        return;
    }

    const chunk_t *current = manager->container->first;
    while (current != NULL) {
        fwrite(current->data, current->element_size, current->size, file);
        current = current->next;
    }

    fclose(file);
}

int cache_exists(const char *cachename) {
    char full_cachename[CACHENAME_LEN + 10];
    sprintf(full_cachename, "../cache/%s", cachename);
    struct stat buffer;
    return stat(full_cachename, &buffer);
}

char *parse_request_to_cachename(const http_request_t *request) {
    char *cachename = malloc(CACHENAME_LEN);
    if (!cachename) {
        perror("Unable to allocate cache name");
        return NULL;
    }
    char *host = parse_host(request->path);
    if (!host) {
        free(cachename);
        return NULL;
    }
    const unsigned long hash = hashn(request->path, HTTP_PATH_LEN);
    sprintf(cachename, "%s_%s_%ld.cch", request->method, host, hash);
    free(host);
    return cachename;
}
