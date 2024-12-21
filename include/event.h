#ifndef _EVENT_H
#define _EVENT_H

#include <stdbool.h>

bool event_init();
// Update the given keypad with the state of each key; Set the second arg to true in case of a quit event
void event_update(bool keypad[16], bool *quit);
void event_destroy();

#endif
