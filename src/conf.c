#include <linux/limits.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include "conf.h"

inline void default_conf(struct batd_conf *conf)
{
	strncpy(conf->smc_path, "/sys/class/power_supply/macsmc-battery",
		sizeof(conf->smc_path));
	conf->charge_limit = 80;
	conf->nwait = 30;
}

static char *read_string(FILE *file, const char *key)
{
	char name[128];
	char val[PATH_MAX];
	char *res = NULL;

	while (fscanf(file, "%127[^=]=%4095[^\n]%*c", name, val) == 2) {
		if (strcmp(name, key) == 0) {
			res = strdup(val);
			break;
		}
	}
	rewind(file);
	return res;
}

static bool read_int(FILE *file, const char *key, int *num)
{
	char *temp = read_string(file, key);
	char *stop;
	*num = strtol(temp, &stop, 10);
	bool ok = stop == NULL || *stop == 0;
	free(temp);
	return ok;
}

int parse_conf(const char *path, struct batd_conf *conf)
{
	FILE *fd = fopen(path, "r");
	int res = -1;
	if (fd == NULL) {
		return -1;
	}
	char *temp = read_string(fd, "smc_path");
	if (temp == NULL) {
		goto cleanup;
	}
	strncpy(conf->smc_path, temp, sizeof(conf->smc_path));
	free(temp);
	res = read_int(fd, "charge_limit", &conf->charge_limit) &&
			      read_int(fd, "nwait", &conf->nwait) ?
		      0 :
		      -1;
cleanup:
	fclose(fd);
	return res;
}
