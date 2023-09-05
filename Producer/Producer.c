#include "Producer.h"

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
