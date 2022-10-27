#ifndef BATD_CONF_H
#define BATD_CONF_H
#include <linux/limits.h>

struct batd_conf {
	char smc_path[PATH_MAX];
	int charge_limit;
	int nwait;
};

// returns a default sane configuration file
extern void default_conf(struct batd_conf *conf);

int parse_conf(const char *path, struct batd_conf *conf);

#endif
