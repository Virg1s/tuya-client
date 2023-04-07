#include <stdlib.h>
#include <string.h>
#include <argp.h>
#include <unistd.h>
#include <syslog.h>
#include <sys/stat.h>
#include <fcntl.h>
#include "tuya_lib.h"

#define BD_MAX_CLOSE 8192

int become_child(void)
{
	switch (fork()) {
	case -1:
		return -1;
	case 0:
		 return 0;
	default:
		_exit(EXIT_SUCCESS); // kodel cia ne exit(0)?
	}
}

int become_daemon(void)
{
	int maxfd, fd;

	if (become_child())
		return -1;

	if (setsid() == -1)
		return -1;

	if (become_child())
		return -1;

	umask(0); //should investigate how exactly musask works
	chdir("/");

	maxfd = sysconf(_SC_OPEN_MAX);
	if (maxfd == -1)
		maxfd = BD_MAX_CLOSE;
	for (fd = 0; fd < maxfd; fd++)
		close(fd);

	close(STDIN_FILENO);

	fd = open("/dev/null", O_RDWR);

	if (fd != STDIN_FILENO)
		return -1;
	if (dup2(STDIN_FILENO, STDOUT_FILENO) != STDOUT_FILENO)
		return -2;
	if (dup2(STDIN_FILENO, STDERR_FILENO) != STDERR_FILENO)
		return -3;

	return 0;
}

static char doc[] =
	"daemon program that sends arbitrary string to tuya cloud\n"
	"(keep in mind that tuya cloud has some funky rules for string parsing)";

static char args_doc[] = "MESSAGE_STRING";

static struct argp_option options[] = {
	{ "device_id", 'd', "STRING", 0, "device id", 0 },
	{ "device_secret", 's', "STRING", 0, "device secret key", 0 },
	{ "product_id", 'p', "STRING", 0, "product id", 0 },
	{ 0 }
};

struct arguments {
	char *args[1];
	char *device_id;
	char *device_secret;
	char *product_id;
};

static error_t parse_opt(int key, char *arg, struct argp_state *state)
{
	struct arguments *arguments = state->input;

	switch (key) {
	case 'd':
		arguments->device_id = arg;
		break;
	case 's':
		arguments->device_secret = arg;
		break;
	case 'p':
		arguments->product_id = arg;
		break;

	case ARGP_KEY_ARG:
		if (state->arg_num >= 1) {
			fputs("\nTOO MANY ARGS\n", stderr);
			argp_usage(state);
		}

		arguments->args[state->arg_num] = arg;

		break;

	default:
		return ARGP_ERR_UNKNOWN;
	}
	return 0;
}

static struct argp argp = { options, parse_opt, args_doc, doc };

int main(int argc, char **argv)
{
	struct arguments arguments = {
		.args = { "default message" },
		.device_secret = "5ZUcwOQRDm3rzNUQ",
		.device_id = "26bf9c459833b88e53mgqj",
		.product_id = "xa5ecaywubiym1bq"
	};

	argp_parse(&argp, argc, argv, 0, 0, &arguments);

	become_daemon();

	//setlogmask(LOG_UPTO (LOG_NOTICE));
	openlog("TUYA MSG", LOG_CONS | LOG_PID | LOG_NDELAY, LOG_LOCAL0);

	syslog(LOG_INFO,
	       "REPORT BEFORE COMMS >>> message: '%s', "
	       "device_id: '%s', device_secret: '%s', product_id: '%s'",
	       arguments.args[0], arguments.device_id, arguments.device_secret,
	       arguments.product_id);

        communicate_with_cloud(arguments.device_id,
		arguments.device_secret,
		arguments.args[0]);

	syslog(LOG_WARNING, "Exiting from the program");

	sleep(20);

	closelog();

	return 0;
}
