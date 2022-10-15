#ifndef BATD_BAT_H
#define BATD_BAT_H

struct threshold_ctx {
	const char *smc_path;
	int charge_limit;
	int delta;
	int nwait;
};

void threshold_loop(const struct threshold_ctx *ctx);

#endif
