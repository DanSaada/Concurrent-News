
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <semaphore.h>
#include <unistd.h>
#include <fcntl.h>


#define MAX_MESSAGE_LENGTH 100
#define NUM_MESSAGE_TYPES 3
#define NUM_CO_EDITORS 3

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

typedef struct {
    char** messages;
    int limitSize;
    int count;
    int in;
    int out;
    sem_t mutex;
    sem_t full;
} UnboundedBuffer;

typedef struct {
    int producerID;
    int numProducts;
    int queueSize;
    BoundedBuffer* buffer;
} Producer;

typedef struct {
    Producer** producers;
    int numProducers;
    UnboundedBuffer* dispatcherQueues;
} Dispatcher;

typedef struct {
    char message[22];
    Dispatcher *dispatcher;
    BoundedBuffer *sharedBuffer;
    int categoryIndex;
} CoEditor;

//----------------GLOBALS------------------
int numProducers;
int coEditorBufferSize;
Producer** producers;
BoundedBuffer* sharedBuffer;
char** messages;


//----------------BOUNDED BUFFER------------------

/**
 * Initializes a bounded buffer with a given size.
 *
 * @param bufferSize The size of the bounded buffer.
 * @return a pointer to a Bounded buffer
 */
BoundedBuffer* initBuffer(int bufferSize) {
    BoundedBuffer* buffer = malloc(sizeof(BoundedBuffer));
    buffer->data = malloc(sizeof(char*)*bufferSize);
    buffer->size = bufferSize;
    buffer->count = 0;
    buffer->in = 0;
    buffer->out = 0;

    // Initialize the mutex semaphore to 1
    sem_init(&buffer->mutex, 0, 1);
    // Initialize the empty semaphore to the buffer size
    sem_init(&buffer->empty, 0, bufferSize);
    // Initialize the full semaphore to 0
    sem_init(&buffer->full, 0, 0);

    return buffer;
}

/**
 * Inserts an article into the bounded buffer.
 * If the buffer is full, the function will block until there is an empty slot available.
 *
 * @param buffer The pointer to the bounded buffer.
 * @param s The string representing the article to be inserted.
 */
void insertBounded(BoundedBuffer* buffer, char* s) {
    // decrements the value of empty by 1 and continues.
    // If the value is 0 (no empty slot available), the thread will be blocked until an empty slot becomes available.
    sem_wait(&buffer->empty);
    //acquiring the mutex semaphore
    sem_wait(&buffer->mutex);

    // critical section
    buffer->data[buffer->in] = strdup(s);
    buffer->in = (buffer->in + 1) % buffer->size;
    buffer->count++;

    // releasing the mutex and allowing other threads to access the buffer.
    sem_post(&buffer->mutex);
    // increments the value of the full semaphore by 1, indicating that there is now one more filled slot in the buffer.
    sem_post(&buffer->full);
}

/**
 * Removes an article from the bounded buffer.
 * If the buffer is empty, the function will block until there is a filled slot available.
 *
 * @param buffer The pointer to the bounded buffer.
 * @return The string representing the removed article.
 */
char* removeBounded(BoundedBuffer* buffer) {
    // decrements the value of full by 1 and continues.
    // If the value is 0 (no filled slot available), the thread will be blocked until a filled slot becomes available.
    sem_wait(&buffer->full);
    // acquiring the mutex
    sem_wait(&buffer->mutex);


    // critical section
    char* s = buffer->data[buffer->out];
    buffer->out = (buffer->out + 1) % buffer->size;
    buffer->count--;
    
    // releasing the mutex and allowing other threads to access the buffer.
    sem_post(&buffer->mutex);
    // increments the value of the empty semaphore by 1, indicating that an empty slot is available in the buffer.
    sem_post(&buffer->empty);
    return s;
}

//----------------UNBOUNDED BUFFER------------------

/**
 * Initializes an unbounded buffer.
 *
 * @param buffer Pointer to the UnboundedBuffer struct to be initialized.
 */
void initUnboundedBuffer(UnboundedBuffer* buffer) {
    // Initialize the buffer's variables
    buffer->messages = malloc(sizeof(char*)*50);
    buffer->limitSize = 50;
    buffer->count = 0;
    buffer->in = 0;
    buffer->out = 0;

    // Initialize the mutex semaphore to ensure thread safety
    sem_init(&buffer->mutex, 0, 1);
    // Initialize the full semaphore to 0 since the buffer is initially empty
    sem_init(&buffer->full, 0, 0);
}

/**
 * Inserts a message into the unbounded buffer.
 *
 * @param buffer Pointer to the UnboundedBuffer struct.
 * @param message The message to be inserted.
 */
void insertUnBounded(UnboundedBuffer* buffer, const char* message) {
    // Wait until there is available space in the buffer
    sem_wait(&buffer->mutex);

    // If the buffer is full, increase its capacity
    if (buffer->count == buffer->limitSize) {
        int newLimitSize = buffer->limitSize * 2;
        char** newMessages = realloc(buffer->messages, sizeof(char*) * newLimitSize);
        // update the next size that should generate a reallocation
        buffer->messages = newMessages;
        buffer->limitSize = newLimitSize;
    }

    // Insert the message into the buffer
    buffer->messages[buffer->in] = strdup(message);
    buffer->in++;
    buffer->count++;

    // Signal that the buffer is not empty
    sem_post(&buffer->mutex);
    sem_post(&buffer->full);

}

/**
 * Removes a message from the unbounded buffer.
 *
 * @param buffer Pointer to the UnboundedBuffer struct.
 * @return The removed message.
 */
char* removeUnBounded(UnboundedBuffer* buffer) {
    // Wait until there is a message available in the buffer
    sem_wait(&buffer->full);
    sem_wait(&buffer->mutex);

    // Remove the message from the buffer
    char* message = buffer->messages[buffer->out];
    buffer->out++;
    buffer->count--;

    // releasing the mutex and allowing other threads to access the buffer.
    sem_post(&buffer->mutex);

    return message;
}


//----------------PRODUCER------------------

/**
 * Creates a producer with the specified ID, number of products, and queue size.
 * The producer object is initialized with the provided values and an internal buffer is created.
 *
 * @param producer Pointer to the Producer struct to be created.
 * @param producerID The ID of the producer.
 * @param numOfProducts The number of products.
 * @param queueSize The size of the producer's queue.
 */
void createProducer(Producer* producer, char* producerID, char* numOfProducts, char* queueSize) {
    producer->producerID = atoi(producerID) - 1;
    producer->numProducts = atoi(numOfProducts);
    producer->queueSize = atoi(queueSize);
    producer->buffer = initBuffer(producer->queueSize);
}

/**
 * Reads the configuration file with the specified filename.
 * The function parses the configuration file, creates producers based on the file contents,
 * and initializes the producers array.
 *
 * @param filename The name of the configuration file to be read.
 */
void readConfigurationFile(const char* filename) {
    FILE* configFile = fopen(filename, "r");

    if (configFile == NULL) {
        printf("Error opening configuration file.\n");
        // Handle error case!!!!!!!!!!!!!!!!!!!!!!!
        return;
    }

    // Initial capacity of the producers array
    int capacity = 10;
    producers = malloc(capacity * sizeof(Producer*));
    numProducers = 0;

    char* line = NULL, *tempLine = NULL, *thirdLine = NULL;
    size_t len = 0, tempLen = 0, thirdLen = 0;
    ssize_t bytesRead;

    // Read the configuration file line by line
    while ((bytesRead = getline(&line, &len, configFile)) != -1) {
        if (bytesRead == 1 && line[0] == '\n') {
            // Skip empty lines
            continue;
        }

        getline(&tempLine, &tempLen, configFile);


        int thirdTempLine = getline(&thirdLine, &thirdLen, configFile);
        //EOF
        if(thirdTempLine == -1){
            // Set the Co-Editor queue size
            coEditorBufferSize = atoi(line);
            break;
        }

        // check for a need of reallocation
        if (numProducers >= capacity) {
            capacity *= 2;
            producers = realloc(producers, capacity * sizeof(Producer*));
        }

        //create and add the new producer
        Producer* producer = malloc(sizeof(Producer));
        createProducer(producer, line, tempLine, thirdLine);
        producers[numProducers] = producer;
        numProducers++;
        
    }
    fclose(configFile);
}

/**
 * Generates articles and inserts them into the bounded buffer.
 *
 * @param arg A void pointer to the index of the Producer.
 * @return A void pointer to indicate the completion of the thread.
 */
void* produce(void* arg) {
    int j = (int)arg;
    char* message = malloc(sizeof(char)* MAX_MESSAGE_LENGTH);
    char* articleTypes[3] = {"SPORTS", "NEWS", "WEATHER"};
    int articleTypeCounter = 0;
    for (int i = 0; i < producers[j]->numProducts; i++) {
        // Determine the article type based on modulo 3 operation
        int typeIndex = i % 3;
        // set the articles
        if (i % 3 == 0 && i != 0){
            articleTypeCounter++;
        }
        // create the message
        snprintf(message, MAX_MESSAGE_LENGTH, "Producer %d %s %d", producers[j]->producerID, articleTypes[typeIndex], articleTypeCounter);
        // insert the article to the buffer
        insertBounded(producers[j]->buffer, message);
       
    }

    insertBounded(producers[j]->buffer, "DONE");
    messages[j] = message;

    return NULL;
}

/**
 * Runs the producer threads.
 * Creates and starts threads for each producer in the producers array.
 *
 * @return Pointer to the array of producer thread IDs (pthread_t).
 */
pthread_t* runProducers() {
    messages = malloc(numProducers * sizeof(char*));

    pthread_t producerThreads[numProducers];
    for (int i = 0; i < numProducers; i++) {

        pthread_create(&producerThreads[i], NULL, produce, (void*)i);
    }
    return producerThreads;

    // Wait for all producer threads to finish
//    for (int i = 0; i < numProducers; i++) {
//        pthread_join(producerThreads[i], NULL);
//    }
}

//----------------DISPATCHER------------------

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

//----------------SCREEN-MANAGER------------------

/**
 * Manages the screen display.
 * Continuously retrieves messages from the shared buffer and prints them to the screen.
 * Keeps track of the number of "DONE" messages received to determine when to exit the loop.
 */
void screenManager() {
    int doneCounter = 0;

    while (doneCounter < 3) {
        char *message = removeBounded(sharedBuffer);
        if (strcmp(message, "DONE") == 0) {
            doneCounter++;
            continue;
        }
        printf("%s\n", message);
    }
}

//----------------CO-EDITOR------------------

/**
 * Initializes a CoEditor instance.
 *
 * @param coEditor           A pointer to the CoEditor instance to initialize.
 * @param dispatcher         A pointer to the Dispatcher instance.
 * @param categoryIndex      The index representing the category of articles that the CoEditor will handle.
 */
void coEditorInit(CoEditor *coEditor, Dispatcher *dispatcher, int categoryIndex) {
    coEditor->dispatcher = dispatcher;
    coEditor->sharedBuffer = sharedBuffer;
    coEditor->categoryIndex = categoryIndex;
}

/**
 * Retrieves messages from the designated unbounded queue of a specific category, "edits them", 
 * and passes them to the shared buffer.
 *
 * @param arg A void pointer to the CoEditor instance.
 * @return    The function returns NULL when the thread exits.
 */
void* coEdit(void* arg) {
    CoEditor* coEditor = (CoEditor*)arg;
    int categoryIndex = coEditor->categoryIndex;
    pthread_t screenManagerThread;
    pthread_create(&screenManagerThread, NULL, screenManager, (void*)&sharedBuffer);

    while (1) {
        // Receive message from Dispatcher queue
        char* message = removeUnBounded(&coEditor->dispatcher->dispatcherQueues[categoryIndex]);

        // Edit the message (block for 0.1 seconds)
        usleep(100000);  // 0.1 seconds
        
        // Check for "DONE" message
        if (strcmp(message, "DONE") == 0) {
            // Pass the "DONE" message without waiting
            insertBounded(coEditor->sharedBuffer, message);
            break;
        }
        
        // Pass the edited message to the shared buffer
        insertBounded(coEditor->sharedBuffer, message);
    }
    return NULL;
}

/**
 * Runs the Co-Editor threads.
 * Creates and starts threads for each Co-Editor in the coEditors array.
 *
 * @param dispatcher Pointer to the Dispatcher object.
 * @return Pointer to the array of Co-Editor thread IDs (pthread_t).
 */
pthread_t* runCoEditors(Dispatcher* dispatcher) {
    pthread_t coEditorThreads[NUM_CO_EDITORS];
    CoEditor coEditors[NUM_CO_EDITORS];

    // create all co-Editor's threads
    for (int i = 0; i < NUM_CO_EDITORS; i++) {
        coEditorInit(&coEditors[i], dispatcher, i);
        pthread_create(&coEditorThreads[i], NULL, coEdit, (void*)&coEditors[i]);
    }


    // Wait for all Co-Editor threads to finish
    for (int i = 0; i < NUM_CO_EDITORS; i++) {
        pthread_join(coEditorThreads[i], NULL);
    }
    printf("DONE\n");
    return coEditorThreads;
}


//----------------CLEAN-UP------------------

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