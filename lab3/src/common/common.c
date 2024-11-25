#include "../../include/common.h"
#include <sys/socket.h>

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
        const ssize_t bytes_sent = send(fd, data + total_sent, size - total_sent, 0);
        if (bytes_sent == -1) {
            perror("send");
            return -1;
        }
        total_sent += bytes_sent;
    }
    return total_sent == size ? 0 : -1;
}
