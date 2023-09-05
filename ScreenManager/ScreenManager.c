#include "ScreenManager.h"

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