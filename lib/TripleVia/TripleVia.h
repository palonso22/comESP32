#ifndef __TRIPLEVIA__H
#define __TRIPLEVIA__H
#include "Arduino.h"
#include "LedIndicator.h"


/* setup inicia vias y tareas */
void setupTripleVia();
void taskScheduler(void*);
#endif