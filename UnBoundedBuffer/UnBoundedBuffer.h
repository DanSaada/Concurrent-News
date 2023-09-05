#ifndef UNBOUNDEDBUFFER_H
#define UNBOUNDEDBUFFER_H

#include <pthread.h>
#include <semaphore.h>

typedef struct {
    char** messages;
    int limitSize;
    int count;
    int in;
    int out;
    sem_t mutex;
    sem_t full;
} UnboundedBuffer;

void initUnboundedBuffer(UnboundedBuffer* buffer);

void insertUnBounded(UnboundedBuffer* buffer, const char* message);

char* removeUnBounded(UnboundedBuffer* buffer);

#endif