#include "../../include/log.h"
#include "../../include/common.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

void logis(const int id, const char *msg) {
    printf("[%d] :: %s\n", id, msg);
}

void logss(const char *id, const char *msg) {
    printf("[%s] :: %s\n", id, msg);
}

void logisi(const int id, const char *msg, const int value) {
    printf("[%d] :: %s %d\n", id, msg, value);
}

void logiss(const int id, const char *msg, const char *value) {
    printf("[%d] :: %s %s\n", id, msg, value);
}

void logssi(const char *id, const char *msg, const int value) {
    printf("[%s] :: %s %d\n", id, msg, value);
}

void logsss(const char *id, const char *msg, const char *value) {
    printf("[%s] :: %s %s\n", id, msg, value);
}

void logurls(const char *url, const char *msg) {
    char * host = parse_host(url);
    printf("[%s] :: %s\n", host, msg);
    free(host);
}
