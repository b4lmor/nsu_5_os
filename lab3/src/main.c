#include <stdio.h>
#include <stdlib.h>

#include "../include/context.h"
#include "../include/server.h"

#define PORT 8080

int main() {
    proxy_context_t *context = init_context(PORT);
    const int err = start_server(context);
    if (err) {
        perror("Error starting server");
        destroy_context(&context);
        return EXIT_FAILURE;
    }
    destroy_context(&context);
    return EXIT_SUCCESS;
}
