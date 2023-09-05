#include "BoundedBuffer.h"

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