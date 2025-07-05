#ifndef SNTP_SERVICE_H
#define SNTP_SERVICE_H

#include <strings.h>

void sntp_service_init(void);
void get_time(char *buffer, size_t max_len);

#endif
