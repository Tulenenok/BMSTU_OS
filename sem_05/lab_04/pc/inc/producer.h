#ifndef PRODUCER_H
#define PRODUCER_H

#include "buffer.h"

void producer_work(buffer_s* const buffer, const int sem_id, const int pro_id);

void producer_create(buffer_s* const buffer, const int pro_id, const int sem_id);

#endif