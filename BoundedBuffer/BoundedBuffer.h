#ifndef BOUNDEDBUFFER_H
#define BOUNDEDBUFFER_H

#include <pthread.h>
#include <semaphore.h>

typedef struct {
    char** data;
    int count; // number of articles currently in the buffer
    int in; // the index where the next element will be inserted in the buffer
    int out; // the index from where the next element will be removed from the buffer
    int size;
    sem_t mutex;
    sem_t empty;
    sem_t full;
} BoundedBuffer;

BoundedBuffer* initBuffer(int bufferSize);

void insertBounded(BoundedBuffer* buffer, char* s);

char* removeBounded(BoundedBuffer* buffer);

#endif