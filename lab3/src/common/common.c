#include "../../include/common.h"
#include <string.h>
#include <ctype.h>
#include <time.h>
#include <unistd.h>
#include <sys/socket.h>
#include <sys/stat.h>

#define BUFFER_SIZE 2048

void bijection_url_filename(const char *path, char *filename) {
    const char *invalid_chars = ":/\\*?\"<>|";
    const char replace_prefix = '_';
    size_t filename_index = 0;
    const size_t path_len = strlen(path);
    for (size_t i = 0; i < path_len; i++) {
        const char current_char = path[i];
        if (strchr(invalid_chars, current_char) != NULL) {
            filename[filename_index++] = replace_prefix;
            filename[filename_index++] = '0' + current_char / 10;
            filename[filename_index++] = '0' + current_char % 10;
        } else {
            filename[filename_index++] = current_char;
        }
    }
    filename[filename_index] = '\0';
}

void bijection_filename_url(const char *filename, char *url) {
    size_t url_index = 0;
    const size_t filename_len = strlen(filename);
    for (size_t i = 0; i < filename_len; i++) {
        const char current_char = filename[i];
        if (current_char == '_') {
            if (i + 2 < filename_len) {
                const char ascii_code_char1 = filename[++i];
                const char ascii_code_char2 = filename[++i];
                const char original_char = (ascii_code_char1 - '0') * 10 + (ascii_code_char2 - '0');
                url[url_index++] = original_char;
            }
        } else {
            url[url_index++] = current_char;
        }
    }
    url[url_index] = '\0';
}

void write_file_to_fd(FILE *file, const int fd, const int skip_lines) {
    char buffer[BUFFER_SIZE] = {0};
    size_t bytes_read;
    for (int i = 0; i < skip_lines; i++) {
        if (fgets(buffer, sizeof(buffer), file) == NULL) {
            if (feof(file)) {
                break;
            }
            perror("Error reading line from file");
            fclose(file);
            return;
        }
    }
    while ((bytes_read = fread(buffer, 1, BUFFER_SIZE, file)) > 0) {
        const ssize_t bytes_written = send(fd, buffer, bytes_read, 0);
        if (bytes_written < 0) {
            perror("Error writing to file descriptor");
            return;
        }
    }
}

void append_file(FILE *src, FILE *dest) {
    if (src == NULL || dest == NULL) {
        fprintf(stderr, "Ошибка: один или оба файла не открыты.\n");
        return;
    }
    char buffer[BUFFER_SIZE];
    size_t bytes_read;
    while ((bytes_read = fread(buffer, sizeof(char), BUFFER_SIZE, src)) > 0) {
        fwrite(buffer, sizeof(char), bytes_read, dest);
    }
}

int check_cache_expiration(const char *path, const long max_diff) {
    FILE *file = fopen(path, "r");
    if (file == NULL) {
        return -1;
    }
    int day, month, year, hour, minute, second;
    if (fscanf(file, "%02d-%02d-%04d %02d:%02d:%02d", &day, &month, &year, &hour, &minute, &second) != 6) {
        fclose(file);
        return -1;
    }
    fclose(file);
    struct tm time_created = {0};
    time_created.tm_mday = day;
    time_created.tm_mon = month - 1;
    time_created.tm_year = year - 1900;
    time_created.tm_hour = hour;
    time_created.tm_min = minute;
    time_created.tm_sec = second;
    const time_t created_time = mktime(&time_created);
    if (created_time == -1) {
        return -1;
    }
    const time_t current_time = time(NULL);
    if (current_time == -1) {
        return -1;
    }
    const double difference_in_seconds = difftime(current_time, created_time);
    const long difference_in_milliseconds = (long)(difference_in_seconds * 1000);
    if (difference_in_milliseconds >= max_diff) {
        remove(path);
        return -1;
    }
    return 0;
}

unsigned long hash(const char *str) {
    unsigned long hash = 5381;
    int c;
    while ((c = *str++)) {
        hash = (hash << 5) + hash + c;
    }
    return hash;
}
