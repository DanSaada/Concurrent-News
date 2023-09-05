#ifndef DISPATCHER_H
#define DISPATCHER_H

#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>

#include "../UnBoundedBuffer/UnBoundedBuffer.h"
#include "../BoundedBuffer/BoundedBuffer.h"
#include "../Producer/Producer.h"

typedef struct {
    Producer** producers;
    int numProducers;
    UnboundedBuffer* dispatcherQueues;
} Dispatcher;

int getMessageType(const char* message);

void initDispatcher(Dispatcher* dispatcher);

void* dispatche(void* arg);

#endif