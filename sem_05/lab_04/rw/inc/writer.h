#ifndef WRITER_H
#define WRITER_H

void writer_create(const int sem_id, const int writer_id);

void writer_work(const int sem_id, const int writer_id);

#endif