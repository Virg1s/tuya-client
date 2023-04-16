#ifndef TUYA_INTERFACE_H
#define TUYA_INTERFACE_H

#include <signal.h>
#include <syslog.h>

extern volatile sig_atomic_t exit_trigger;
extern void (*log_function)(int log_level, const char *format_string, ...);
void set_exit_trigger(int signal);

#endif
