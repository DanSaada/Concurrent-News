#ifndef PRODUCER_H
#define PRODUCER_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>  
#include "../BoundedBuffer/BoundedBuffer.h"

typedef struct {
    int producerID;
    int numProducts;
    int queueSize;
    BoundedBuffer* buffer;
} Producer;

void createProducer(Producer* producer, char* producerID, char* numOfProducts, char* queueSize);

void readConfigurationFile(const char* filename);

void* produce(void* arg);

pthread_t* runProducers();

#endif