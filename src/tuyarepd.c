#include <stdlib.h>
#include <string.h>
#include <argp.h>
#include <unistd.h>
#include <syslog.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <signal.h>
#include <stdarg.h>
#include "tuya_interface.h"
#include "tuyarepd.h"

#define STDOUT_LOG_INTRO_MAX_SIZE 80
#define BD_MAX_CLOSE 8192
#define OPT_DEV_ID 'd'
#define OPT_DEV_SECR 's'
#define OPT_PROD_ID 'p'
#define OPT_DAEMONIZE 0x80

static char doc[] =
	"daemon program that sends arbitrary string to tuya cloud\n"
	"(keep in mind that tuya cloud has some funky rules for string parsing)";

static char args_doc[] = "MESSAGE_STRING";

static struct argp_option options[] = {
	{ "device_id", OPT_DEV_ID, "STRING", 0, "device id", 0 },
	{ "device_secret", OPT_DEV_SECR, "STRING", 0, "device secret key", 0 },
	{ "product_id", OPT_PROD_ID, "STRING", 0, "product id", 0 },
	{ "daemonize", OPT_DAEMONIZE, 0, 0, "run program as daemon", 0 },
	{ 0 }
};

struct arguments {
	int daemon;
	char *args[1];
	char *device_id;
	char *device_secret;
	char *product_id;
};

static error_t parse_opt(int key, char *arg, struct argp_state *state)
{
	struct arguments *arguments = state->input;

	switch (key) {
	case OPT_DEV_ID:
		arguments->device_id = arg;
		break;
	case OPT_DEV_SECR:
		arguments->device_secret = arg;
		break;
	case OPT_PROD_ID:
		arguments->product_id = arg;
		break;
	case OPT_DAEMONIZE:
		arguments->daemon = 1;
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

static struct argp argp = {options, parse_opt, args_doc, doc, NULL, NULL, NULL};

volatile sig_atomic_t exit_trigger = 0;

void set_exit_trigger(int signal)
{
	(void)signal;
	exit_trigger = 1;
}

void (*log_function)(int log_level, const char *format_string, ...);

void log_stdout(int log_level, const char *format_string, ...)
{
	(void) log_level;

	va_list args;
	va_start(args, format_string);
	vprintf(format_string, args);
	va_end(args);
	fflush(stdout);
}

void log_syslog(int log_level, const char *format_string, ...)
{
	va_list args;
	va_start(args, format_string);
	vsyslog(log_level, format_string, args);
	va_end(args);
}

void log_stderr(int log_level, const char *format_string, ...)
{
	(void) log_level;

	va_list args;
	va_start(args, format_string);
	vfprintf(stderr, format_string, args);
	va_end(args);
}

int become_child(void)
{
	switch (fork()) {
	case -1:
		return -1;
	case 0:
		return 0;
	default:
		exit(0);
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

	umask(0);
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

int initialize_resources(struct arguments *args)
{
	int ret_val = 0;

	signal(SIGINT, set_exit_trigger);
	signal(SIGTERM, set_exit_trigger);
	signal(SIGHUP, set_exit_trigger);

	log_function = log_stdout;

	if (args->daemon) {
		ret_val |= become_daemon();

		ret_val |= setlogmask(LOG_UPTO (LOG_DEBUG));

		openlog("TUYA MSG", LOG_CONS | LOG_PID | LOG_NDELAY, LOG_LOCAL0);

		log_function = log_syslog;
	}


	return ret_val;
}

int release_resources(void)
{
	closelog();
	
	return 0;
}

int main(int argc, char **argv)
{
	int ret_val;

	struct arguments arguments = {
	       .args = { "default message" },
	       .daemon = 0,
	       .device_secret = "",
	       .device_id = "",
	       .product_id = ""
	};

	argp_parse(&argp, argc, argv, 0, 0, &arguments);

	//setting to stderr in case something fails in next function
	log_function = log_stderr; 

	ret_val = initialize_resources(&arguments);

	if (ret_val) {
		log_function(LOG_WARNING, "failed to initialize resources, exiting");
		goto cleanup;
	}

	ret_val = communicate_with_cloud(arguments.device_id, arguments.device_secret,
			       arguments.args[0]);

	log_function(LOG_INFO, "Exiting from the program");
	
	cleanup:

	release_resources();

	return 0;
}
