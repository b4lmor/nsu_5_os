#ifndef SERVER_H
#define SERVER_H

#define CONNECTIONS_NUMBER 5

#include "context.h"

int start_server(int port, Context *context);

#endif
