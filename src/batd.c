#include <fcntl.h>
#include <linux/limits.h>
#include <signal.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include "debug.h"
#include "conf.h"
#include "batd.h"

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

static void get_charge_behaviour_path(const struct batd_conf *t, char *path,
				      size_t n)
{
	strncpy(path, t->smc_path, n);
	strcat(path, "/charge_behaviour");
}

static int get_charging_state(const struct batd_conf *t, bool *is_enabled)
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
		*is_enabled = true;
	} else if (strstr(state, "inhibit-charge") != NULL) {
		*is_enabled = false;
	} else {
		return -1;
	}
	return 0;
}

static int set_charging_state(const struct batd_conf *t, const char *arg)
{
	char path[PATH_MAX];
	get_charge_behaviour_path(t, path, sizeof(path));
	return nwrite(path, arg, strlen(arg) + 1) == -1 ? -1 : 0;
}

static int disable_charging(const struct batd_conf *arg)
{
	return set_charging_state(arg, "inhibit-charge\n");
}

static int enable_charging(const struct batd_conf *arg)
{
	return set_charging_state(arg, "auto\n");
}

static int get_current_charge(const struct batd_conf *arg)
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

// exit gracefully
static void sigterm_handler(int sig)
{
	_exit(0);
}

volatile sig_atomic_t g_reload_config = true;

// reload our configuration
static void sighup_handler(int sig)
{
	g_reload_config = true;
}

// will only return if it got interrupted by a signal
static void threshold_loop_inner(const struct batd_conf *conf)
{
	bool is_charging_enabled = false;
	get_charging_state(conf, &is_charging_enabled);
	debug_printf(LOG_INFO, "default charging state: %s\n",
		     is_charging_enabled ? "enabled" : "disabled");
	do {
		int charge = get_current_charge(conf);
		if (charge >= conf->charge_limit) {
			if (is_charging_enabled &&
			    disable_charging(conf) == 0) {
				is_charging_enabled = false;
				debug_printf(
					LOG_INFO,
					"charging disabled at %d: capacity\n",
					charge);
			}
		} else if (is_charging_enabled && enable_charging(conf) == 0) {
			// enable charging so that we can charge to the limit
			is_charging_enabled = true;
			debug_printf(LOG_INFO,
				     "charging enabled at %d: capacity\n",
				     charge);
		}
	} while (!g_reload_config && sleep(conf->nwait) == 0);
	// reaches here if it got interrupted by a signal
}

static int threshold_loop(void)
{
	do {
		struct batd_conf conf;
		if (g_reload_config) {
			const char *conf_path = "";
			if (parse_conf(conf_path, &conf) != 0) {
				debug_printf(LOG_ERR,
					     "error parsing config file: %s\n",
					     conf_path);
				return -1;
			}
			g_reload_config = false;
		}

		threshold_loop_inner(&conf);
	} while (true);
	return 0;
}

int batd(void)
{
	signal(SIGTERM, sigterm_handler);
	signal(SIGHUP, sighup_handler);
	return threshold_loop();
}
