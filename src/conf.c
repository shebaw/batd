#include "conf.h"

int parse_conf(const char *path, struct batd_conf *conf)
{
	conf->smc_path = "/sys/class/power_supply/macsmc-battery";
	conf->charge_limit = 80;
	conf->nwait = 30;
	return 0;
}
