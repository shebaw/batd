#include <stdbool.h>
#include <unistd.h>
#include <fcntl.h>
#include <stdlib.h>
#include <string.h>
#include "bat.h"

static ssize_t nread(const char *path, char *buf, size_t n)
{
	int fd;
	ssize_t nread;
	fd = open(path, O_RDONLY);
	if (fd == -1) {
		return -1;
	}
	nread = read(fd, buf, n);
	close(fd);
	return nread;
}

static void get_charge_behaviour_path(const struct threshold_ctx *t, char *path, size_t n)
{
	strncpy(path, t->smc_path, n);
	strcat(path, "/charge_behaviour");
}

static int get_charging_state(const struct threshold_ctx *t, bool *is_disabled)
{
	char path[200];
	char state[20];
	ssize_t n;

	get_charge_behaviour_path(t, path, sizeof(path));
	n = nread(path, state, sizeof(state));
	if (n == -1) {
		return -1;
	}
	if (strcmp(state, "auto") == 0) {
		*is_disabled = false;
	} else if (strcmp(state, "inhibit-charge") == 0) {
		*is_disabled = true;
	} else {
		return -1;
	}
	return 0;
}

static int set_charging_state(const struct threshold_ctx *t, const char *arg)
{
	char path[200];
	int fd;
	int written;

	get_charge_behaviour_path(t, path, sizeof(path));

	fd = open(path, O_WRONLY | O_CREAT | O_TRUNC,
	   S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH);
	if (fd == -1) {
		return -1;
	}
	written = write(fd, arg, strlen(arg) + 1);
	close(fd);
	return written == -1 ? -1 : 0;
}

static int disable_charging(const struct threshold_ctx *arg)
{
	return set_charging_state(arg, "inhibit-charge");
}

static int enable_charging(const struct threshold_ctx *arg)
{
	return set_charging_state(arg, "auto");
}

static int get_current_charge(const struct threshold_ctx *arg)
{
	char path[200];
	char s[10];
	strncpy(path, arg->smc_path, sizeof(path));
	strcat(path, "/cycle_count");
	if (nread(path, s, sizeof(s)) == -1) {
		return 0;
	}
	return atoi(s);
}

void threshold_loop(const struct threshold_ctx *ctx)
{
	bool is_charging_disabled = false;
	get_charging_state(ctx, &is_charging_disabled);
	for (;; sleep(ctx->nwait)) {
		int charge;

		charge = get_current_charge(ctx);
		if (ctx->charge_limit - charge < ctx->delta) {
			if (!is_charging_disabled) {
				disable_charging(ctx);
				is_charging_disabled = true;
			}
			continue;
		}

		if (is_charging_disabled) {
			// enable charging
			enable_charging(ctx);
			is_charging_disabled = false;
		}

	}
}
