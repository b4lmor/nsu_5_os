#ifndef COMMON_H
#define COMMON_H

#include <stdio.h>

void bijection_url_filename(const char *path, char *filename);

void bijection_filename_url(const char *filename, char *url);

void write_file_to_fd(FILE *file, int fd, int skip_lines);

void append_file(FILE *src, FILE *dest);

int check_cache_expiration(const char *path, long max_diff);

unsigned long hash(const char *str);

#endif
