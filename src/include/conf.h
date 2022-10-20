#ifndef BATD_CONF_H
#define BATD_CONF_H

struct batd_conf {
	const char *smc_path;
	int charge_limit;
	int nwait;
};

int parse_conf(const char *path, struct batd_conf *conf);

#endif
