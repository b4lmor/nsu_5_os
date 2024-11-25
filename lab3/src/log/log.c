#include "../../include/log.h"

#include <stdio.h>

void logi(const int id, const char *msg) {
    printf("[%d] :: %s\n", id, msg);
}

void logs(const char *id, const char *msg) {
    printf("[%32s] :: %s\n", id, msg);
}
