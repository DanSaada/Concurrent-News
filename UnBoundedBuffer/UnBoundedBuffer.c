#include "UnBoundedBuffer.h"

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