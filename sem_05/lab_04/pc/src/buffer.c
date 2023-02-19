#include "../inc/buffer.h"
#include <string.h>
 
int init_buffer(buffer_s* const buffer) 
{
    if (NULL == buffer) 
        return -1;

    buffer->cptr = ALPHABET;
    buffer->pptr = ALPHABET;

    return 0;
}

int write_buffer(buffer_s *const buffer)
{
    if (NULL == buffer)
        return -1;

    buffer->pptr++;
    return 0;
}

int read_buffer(buffer_s* const buffer, char *const dest) 
{
    if (NULL == buffer)
        return -1;

    *dest = *(buffer->cptr++);
    return 0;
}