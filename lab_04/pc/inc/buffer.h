#ifndef BUFFER_H
#define BUFFER_H

#define ALPHABET "abcdefghijklmnopqrstuvwxyzabcdefghijklmnopqrstuvwxyz"

typedef struct 
{
    char *cptr;    // указатель потребителей
    char *pptr;    // указатель производителей
} buffer_s;

int init_buffer(buffer_s* const buffer);
int write_buffer(buffer_s* const buffer);
int read_buffer(buffer_s* const buffer, char* const dest);

#endif