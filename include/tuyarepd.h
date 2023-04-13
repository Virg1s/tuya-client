#ifndef TUYA_INTERFACE_H
#define TUYA_INTERFACE_H

#include <signal.h>

extern volatile sig_atomic_t exit_trigger;
void set_exit_trigger(int signal);

#endif
