#ifndef COMMON_H
#define COMMON_H

#include <stdio.h>

int cas(char *value, char expected, char replaceto);

unsigned int hashn(const char *str, size_t max_size);

int send_data_to_fd(int fd, const void *data, size_t size);

char *parse_host(const char *url);

#endif
