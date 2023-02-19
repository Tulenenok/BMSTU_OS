#ifndef CONSUMER_H
#define CONSUMER_H

#include "buffer.h"

void consumer_work(buffer_s* const buffer, const int sem_id, const int con_id);

void consumer_create(buffer_s* const buffer, const int con_id, const int sem_id);

#endif