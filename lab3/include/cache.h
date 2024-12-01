#ifndef CACHE_H
#define CACHE_H

#include "subscription.h"
#include "http_utils.h"

#define CACHE_BUFFER_SIZE 4048

void share_cache(subscription_manager_t *manager, const char* cachename, proxy_context_t *proxy_context);

void save_cache(const subscription_manager_t *manager, const char *cachename);

int cache_exists(const char *cachename);

char *parse_request_to_cachename(const http_request_t *request);

#endif
