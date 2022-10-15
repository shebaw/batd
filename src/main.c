#include "bat.h"
#include "daemonize.h"

int main(int argc, char *argv[])
{

	struct threshold_ctx ctx = {
		.smc_path = "/sys/class/power_supply/macsmc-battery",
		.charge_limit = 80,
		.delta = 2,
		.nwait = 30
	};

	threshold_loop(&ctx);
	daemonize();
	return 0;
}
