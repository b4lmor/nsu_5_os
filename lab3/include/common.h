#ifndef COMMON_H
#define COMMON_H

#include <stdio.h>

unsigned int hashn(const char *str, size_t max_size);

int send_data_to_fd(int fd, const void *data, size_t size);

#endif
