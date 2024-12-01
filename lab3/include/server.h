#ifndef SERVER_H
#define SERVER_H

#define PENDING_CONNECTIONS_NUMBER 5

#include "context.h"

int start_server(proxy_context_t *context);

#endif
