#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <semaphore.h>
#include <unistd.h>
#include <fcntl.h>

#include "../UnboundedBuffer/UnBoundedBuffer.h"
#include "../BoundedBuffer/BoundedBuffer.h"
#include "../Producer/Producer.h"
#include "../Dispatcher/Dispatcher.h"
#include "../CoEditor/CoEditor.h"
#include "../ScreenManager/ScreenManager.h"

#define MAX_MESSAGE_LENGTH 100
#define NUM_MESSAGE_TYPES 3
#define NUM_CO_EDITORS 3

//----------------GLOBALS------------------
int numProducers;
int coEditorBufferSize;
Producer** producers;
BoundedBuffer* sharedBuffer;
char** messages;


void freeProducers() {
    // Free the messages produced by the producers
    for (int i = 0; i < numProducers; i++) {
        free(messages[i]);
    }

    // Free the messages array
    free(messages);

    // Free the memory for each producer
    for (int i = 0; i < numProducers; i++) {
        // Free the buffer data
        for (int j = 0; j < producers[i]->buffer->size; j++) {
            free(producers[i]->buffer->data[j]);
        }
        free(producers[i]->buffer->data);

        // Destroy the semaphores
        sem_destroy(&producers[i]->buffer->mutex);
        sem_destroy(&producers[i]->buffer->empty);
        sem_destroy(&producers[i]->buffer->full);

        // Free the buffer itself
        free(producers[i]->buffer);

        // Free the producer
        free(producers[i]);
    }

    // Free the array of producers
    free(producers);
}

void freeDispatcher(Dispatcher* dispatcher) {
    // Free the unbounded queues of the sorted articles
    for (int i = 0; i < NUM_MESSAGE_TYPES; i++) {
        UnboundedBuffer* buffer = &dispatcher->dispatcherQueues[i];
        free(buffer->messages);
        sem_destroy(&buffer->mutex);
        sem_destroy(&buffer->full);
    }

    // Free the dispatcher queues array
    free(dispatcher->dispatcherQueues);
}

void freeSharedBuffer(BoundedBuffer* buffer) {
    free(buffer->data);
    sem_destroy(&buffer->mutex);
    sem_destroy(&buffer->empty);
    sem_destroy(&buffer->full);
    free(buffer);
}

void cleanUp(Dispatcher* dispatcher, BoundedBuffer* sharedBuffer) {
    freeProducers();
    freeDispatcher(dispatcher);
    freeSharedBuffer(sharedBuffer);
}


/**
 * Implements the logic of the program:
 * - Create and run the producers for to generate messages.
 * - Creates and initializes the dispatcher to extract the messages from the producers and insert
 *   them to the corresponding coeditor queue by their article subject.
 * - Creates and runs the Co-Editors to remove the messages from the sorted unbounded queues, and
 *   insert them into the last shared bounded buffer, which they would be extract from by the screen
 *   manager and would be printed to the screen.
 */
void programLogic() {
     pthread_t* producerThreads = runProducers();

    // Create the dispatcher and initialize it with the producer queues
    Dispatcher dispatcher;
    dispatcher.dispatcherQueues =(UnboundedBuffer *) malloc(sizeof(UnboundedBuffer)*3);
    initDispatcher(&dispatcher);
    // Run the dispatcher logic

    pthread_t dispatcherThread;
    pthread_create(&dispatcherThread, NULL, dispatche, (void*)&dispatcher);
    pthread_join(dispatcherThread, NULL);
    
    // create the last bounded shared buffer
    sharedBuffer = initBuffer(coEditorBufferSize);
    pthread_t* coEditorThreads = runCoEditors(&dispatcher);
    
    // free all allocated memory
    cleanUp(&dispatcher, sharedBuffer);
}

int main(int argc, char* argv[]) {
    if (argc != 2) {
        return 1;
    }

    const char* configFile = argv[1];

    readConfigurationFile(configFile);

    programLogic();

    return 0;
}