/*
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of the
 * License, or (at your option) any later version.
 *
 * tn.razy@gmail.com
 */

#include "log.h"
#include "config.h"
#include "ui.h"

#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>
#include <signal.h>
#include <getopt.h>

#define VERSION 					"0.1"

static void usage();

static void version();

static void test_config(char *filename);

int main(int argc, const char **argv)
{
	char *cfg_fileanme = NULL;

	const char short_options[] = "dvhtc:";
	const struct option long_options[] = 
	{
		{"help", 	no_argument, 		NULL, 	'h'},
		{"debug", 	no_argument, 		NULL, 	'd'},
		{"version", 	no_argument, 		NULL, 	'v'},
		{"test", 	no_argument, 		NULL, 	't'},
		{"config", 	optional_argument, 	NULL, 	'c'},
		{NULL, 		0, 			NULL, 	 0 }
	};

	opterr = 0;

	register int opt_next;

	while(opt_next = getopt_long(argc, (char * const *)argv, short_options, long_options, NULL), opt_next != -1)
	{
		switch(opt_next)
		{
			case 'h':
				usage();
				break;

			case 'v':
				version();
				break;

			case 'd':
				cfg_set_debug();
				break;

			case 't':
				test_config(cfg_fileanme);
				break;

			case 'c':
				if(optarg && strlen(optarg) != 0)
				{
					cfg_fileanme = optarg;
				}
				break;
		}
	}

	cfg_load(cfg_fileanme);

	if(!cfg_get_debug())
	{
		printf("Run mpdcoverart in daemon ...\n");

		if(signal(SIGHUP, SIG_IGN) == SIG_ERR)
		{
			die("Regist SIGHUP error: %s", strerror(errno));
		}

		switch(fork())
		{
			case -1:
				die("fork() error: %s", strerror(errno));
				break;

			case 0:
				break;

			default:
				exit(EXIT_SUCCESS);
		}

		/* detach console */
		if(setsid() < 0)
		{
			die("Failed to detach console");
		}

		if(chdir("/") < 0)
		{
			die("Failed to change working directory: %s", strerror(errno));
		}

		close(STDIN_FILENO);
		close(STDOUT_FILENO);
		close(STDERR_FILENO);
	}

	char *skin_name = cfg_get_skinname();

	ui_skin_load(skin_name);

	free(skin_name);

	ui_load();

	return EXIT_SUCCESS;
}

static void version()
{
	printf("Mpdcoverart %s\n", VERSION);

	exit(EXIT_SUCCESS);
}

static void usage()
{
	fprintf(stderr, "Usage:\n");
	fprintf(stderr, "\tmpdcoverart [OPTIONS...]\n");
	fprintf(stderr, "\n\n");

	fprintf(stderr, "Get cover art from douban.com.\n");
	fprintf(stderr, "Lastest version: \033[1;4mhttp://github.com/tnrazy/mpdcovrtart\033[0m\n");
	fprintf(stderr, "\n\n");

	fprintf(stderr, "Options:\n");
	fprintf(stderr, "\t-h, --help\t\t\tshow help options.\n");
	fprintf(stderr, "\t-v, --version\t\t\tshow version.\n");
	fprintf(stderr, "\t-d, --debug\t\t\tdebug.\n");
	fprintf(stderr, "\t-t, --test\t\t\tcheck config.\n");
	fprintf(stderr, "\t-c, --config [FILENAME]\t\tspecified cofnig file.\n");

	fprintf(stderr, "\n");

	exit(EXIT_SUCCESS);
}

static void test_config(char *filename)
{
	cfg_set_debug();

	cfg_load(filename);

	exit(EXIT_SUCCESS);
}
