#ifndef TUYA_INTERFACE_H
#define TUYA_INTERFACE_H

#include <signal.h>

extern volatile sig_atomic_t exit_trigger;
void set_exit_trigger(int signal);
/*
static struct argp_option options[] = {
struct arguments {
static error_t parse_opt(int key, char *arg, struct argp_state *state)
static struct argp argp = { options, parse_opt, args_doc, doc };
int main(int argc, char **argv)
*/

#endif
