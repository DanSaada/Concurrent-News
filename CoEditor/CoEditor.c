#include "CoEditor.h"

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