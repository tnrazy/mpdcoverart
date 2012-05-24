/*
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of the
 * License, or (at your option) any later version.
 *
 * tn.razy@gmail.com
 */

#include "msg.h"
#include "ui.h"
#include "setting.h"

#include <stdlib.h>
#include <unistd.h>
#include <signal.h>

int main(int argc, const char **argv)
{
	char *skin_name;

	if(signal(SIGHUP, SIG_IGN) == SIG_ERR)
		die("signal() error.");

	skin_name = cfg_get_skinname();

	switch(fork())
	{
		case -1:
			die("Fork error.");
			break;
		case 0:
			break;
		default:
			exit(0);
	}

	setsid();

	chdir("/");

	ui_skin_load(skin_name);

	free(skin_name);

	ui_load();

	return 0;
}