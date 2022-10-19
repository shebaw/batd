#include <stdbool.h>
#include <unistd.h>
#include <fcntl.h>
#include <stdlib.h>
#include <string.h>
#include <linux/limits.h>
#include "debug.h"
#include "bat.h"

static ssize_t nread(const char *path, char *buf, size_t n)
{
	int fd;
	ssize_t nread;
	fd = open(path, O_RDONLY);
	if (fd == -1) {
		debug_printf(LOG_ERR, "open failed on: %s\n", path);
		return -1;
	}
	nread = read(fd, buf, n);
	close(fd);
	if (nread == -1) {
		debug_printf(LOG_ERR, "read failed on: %s\n", path);
	}
	return nread;
}

static ssize_t nwrite(const char *path, const void *buf, size_t n)
{
	int fd;
	ssize_t written;
	fd = open(path, O_WRONLY | O_CREAT | O_TRUNC,
	   S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH);
	if (fd == -1) {
		debug_printf(LOG_ERR, "open failed on: %s\n", path);
		return -1;
	}
	written = write(fd, buf, n);
	close(fd);
	if (written == -1) {
		debug_printf(LOG_ERR, "write failed on: %s\n", path);
	}
	return written;
}

static void get_charge_behaviour_path(const struct threshold_ctx *t, char *path, size_t n)
{
	strncpy(path, t->smc_path, n);
	strcat(path, "/charge_behaviour");
}

static int get_charging_state(const struct threshold_ctx *t, bool *is_disabled)
{
	char path[PATH_MAX];
	char state[20];
	ssize_t n;

	get_charge_behaviour_path(t, path, sizeof(path));
	n = nread(path, state, sizeof(state));
	if (n == -1) {
		return -1;
	}
	if (strstr(state, "auto") != NULL) {
		*is_disabled = false;
	} else if (strstr(state, "inhibit-charge") != NULL) {
		*is_disabled = true;
	} else {
		return -1;
	}
	return 0;
}

static int set_charging_state(const struct threshold_ctx *t, const char *arg)
{
	char path[PATH_MAX];
	get_charge_behaviour_path(t, path, sizeof(path));
	return nwrite(path, arg, strlen(arg) + 1) == -1 ? -1 : 0;
}

static int disable_charging(const struct threshold_ctx *arg)
{
	return set_charging_state(arg, "inhibit-charge\n");
}

static int enable_charging(const struct threshold_ctx *arg)
{
	return set_charging_state(arg, "auto\n");
}

static int get_current_charge(const struct threshold_ctx *arg)
{
	char path[PATH_MAX];
	char s[10];
	strncpy(path, arg->smc_path, sizeof(path));
	strcat(path, "/capacity");
	if (nread(path, s, sizeof(s)) == -1) {
		return 0;
	}
	return atoi(s);
}

void threshold_loop(const struct threshold_ctx *ctx)
{
	bool is_charging_disabled = false;
	get_charging_state(ctx, &is_charging_disabled);
	debug_printf(LOG_INFO, "default charging state: %s\n", is_charging_disabled ? "disabled" : "enabled");
	for (;; sleep(ctx->nwait)) {
		int charge = get_current_charge(ctx);
		if (charge >= ctx->charge_limit) {
			if (!is_charging_disabled && disable_charging(ctx) == 0) {
				is_charging_disabled = true;
				debug_printf(LOG_INFO, "charging disabled at %d: capacity\n", charge);
			}
		} else if (is_charging_disabled && enable_charging(ctx) == 0) {
			// enable charging so that we can charge to the limit
			is_charging_disabled = false;
			debug_printf(LOG_INFO, "charging enabled at %d: capacity\n", charge);
		}
	}
}
