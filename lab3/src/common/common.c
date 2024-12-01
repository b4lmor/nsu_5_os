#include "../../include/common.h"

#include <stdatomic.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>

int cas(char *value, char expected, char replaceto) {
    return atomic_compare_exchange_strong(value, &expected, replaceto);
}

unsigned int hashn(const char *str, const size_t max_size) {
    unsigned long hash = 5381;
    int c;
    size_t i = 0;
    while (((c = *str++)) && i < max_size) {
        hash = (hash << 5) + hash + c;
        i++;
    }
    return hash;
}

int send_data_to_fd(const int fd, const void *data, const size_t size) {
    size_t total_sent = 0;
    while (total_sent < size) {
        const ssize_t bytes_sent = send(fd, data + total_sent, size - total_sent, MSG_NOSIGNAL);
        if (bytes_sent == -1) {
            perror("send");
            return -1;
        }
        total_sent += bytes_sent;
    }
    return total_sent == size ? 0 : -1;
}

char *parse_host(const char *url) {
    const char *host_start = strstr(url, "://");
    if (host_start) {
        host_start += 3;
    } else {
        host_start = url;
    }
    const char *host_end = strchr(host_start, '/');
    const size_t host_length = host_end ? host_end - host_start : strlen(host_start);
    char *host = malloc(host_length + 1);
    if (!host) {
        perror("malloc parse host");
        return NULL;
    }
    strncpy(host, host_start, host_length);
    host[host_length] = '\0';
    return host;
}
