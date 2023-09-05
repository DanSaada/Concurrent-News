#include "Dispatcher.h"

/**
 * Extracts the message type from the message string and returns the corresponding message type number.
 *
 * @param message   The message string from which to extract the message type.
 * @return The message type number: 0 for "SPORTS", 
 *                                  1 for "NEWS", 
 *                                  2 for "WEATHER",
 *                                  -1 for unknown message type.
 */
int getMessageType(const char* message) {

    // Compare the message with predefined strings
    if (strstr(message, "SPORTS") != NULL) {
        return 0;  // Sports message type
    } else if (strstr(message, "NEWS") != NULL) {
        return 1;  // News message type
    } else if (strstr(message, "WEATHER") != NULL) {
        return 2;  // Weather message type
    } else {
        // Handle unrecognized message types
        return -1;
    }
}

/**
 * Initializes the Dispatcher structure and sets up the references to the producer queues and dispatcher queues.
 *
 * @param dispatcher    Pointer to the Dispatcher structure to be initialized.
 */
void initDispatcher(Dispatcher* dispatcher) {
    // connect between the dispatcher and the producer's queue.
    dispatcher->producers = producers;
    dispatcher->numProducers = numProducers;
    // intialize the unbounded queues of the sorted articles.
    for (int i = 0; i < NUM_MESSAGE_TYPES; i++) {
        initUnboundedBuffer(&dispatcher->dispatcherQueues[i]);
    }
}

/**
 * The dispatcher function continuously scans the producer queues using a Round Robin algorithm and sorts the received messages
 * based on their types into the corresponding dispatcher queues.
 * Once a "DONE" message is received from all producers, it sends a "DONE" message through each dispatcher queue.
 *
 * @param dispatcher    Pointer to the Dispatcher structure.
 */
void* dispatche(void* arg) {
Dispatcher* dispatcher = (Dispatcher*)arg;
    int doneCounter = 0;  // Counter to keep track of "DONE" messages received from Producers
    while (doneCounter < numProducers) {
        for (int i = 0; i < numProducers; i++) {
            if(dispatcher->producers[i]->buffer->count > 0) {
                char* message = removeBounded(dispatcher->producers[i]->buffer);

                if (strcmp(message, "DONE") == 0) {
                    doneCounter++;
                } else {
                    int messageType = getMessageType(message);
                    // check for a valid article type
                    if (messageType == -1){continue;}
                    insertUnBounded(&dispatcher->dispatcherQueues[messageType], message);
                    
                }
            }
//            free(message);
        }
    }
    // Send "DONE" message through each Dispatcher queue
    for (int i = 0; i < NUM_MESSAGE_TYPES; i++) {
        insertUnBounded(&dispatcher->dispatcherQueues[i], "DONE");
    }
    return NULL;
}