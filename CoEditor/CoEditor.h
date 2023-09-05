#ifndef COEDITOR_H
#define COEDITOR_H

#include <stdio.h>
#include <string.h>
#include <unistd.h> 

#include "../UnBoundedBuffer/UnBoundedBuffer.h"
#include "../BoundedBuffer/BoundedBuffer.h"
#include "../Dispatcher/Dispatcher.h"

typedef struct {
    char message[22];
    Dispatcher *dispatcher;
    BoundedBuffer *sharedBuffer;
    int categoryIndex;
} CoEditor;

void coEditorInit(CoEditor *coEditor, Dispatcher *dispatcher, int categoryIndex);

void* coEdit(void* arg);

pthread_t* runCoEditors(Dispatcher* dispatcher);

#endif