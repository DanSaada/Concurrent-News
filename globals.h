#define MAX_MESSAGE_LENGTH 100
#define NUM_MESSAGE_TYPES 3
#define NUM_CO_EDITORS 3

//----------------GLOBALS------------------
int numProducers;
int coEditorBufferSize;
Producer** producers;
BoundedBuffer* sharedBuffer;
char** messages;

