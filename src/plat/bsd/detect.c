/* standard includes */
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <unistd.h>
#include <getopt.h>

/* BSD-specific includes */
#if defined(__FreeBSD__)
	#include <sys/utsname.h>
	#include <time.h>
#elif defined(__NetBSD__)
	#include <sys/utsname.h>
#elif defined(__OpenBSD__)
	#define OS OPENBSD
	#include <sys/utsname.h>
	#include <time.h>
#elif defined(__DragonFly__)
	#include <sys/utsname.h>
	#include <time.h>
#endif

/* program includes */
#include "../../misc.h"
#include "../../disp.h"
#include "../../util.h"

/*	detect_distro
	detects the computer's distribution (really only relevant on Linux)
	argument char *str: the char array to be filled with the distro name
*/
void detect_distro(char *str, bool error)
{
	FILE *distro_file;

	distro_file = popen("uname -sr | tr -d '\\n'", "r");
	fgets(str, MAX_STRLEN, distro_file);
	pclose(distro_file);

	return;
}

/*	detect_arch
	detects the computer's architecture
	argument char *str: the char array to be filled with the architecture
*/
void detect_arch(char *str)
{
	struct utsname arch_info;
	uname(&arch_info);
	safe_strncpy(str, arch_info.machine, MAX_STRLEN);

	return;
}

/*	detect_host
	detects the computer's hostname and active user and formats them
	argument char *str: the char array to be filled with the user and hostname in format 'user@host'
*/
void detect_host(char *str)
{
	char *given_user = "Unknown";
	char given_host[MAX_STRLEN] = "Unknown";

	given_user = getlogin(); /* getlogin is apparently buggy on linux, so this might be changed */

	struct utsname host_info;
	uname(&host_info);
	safe_strncpy(given_host, host_info.nodename, MAX_STRLEN);

	snprintf(str, MAX_STRLEN, "%s@%s", given_user, given_host);

	return;
}

/*	detect_kernel
	detects the computer's kernel
	argument char *str: the char array to be filled with the kernel name
*/
void detect_kernel(char *str)
{
	struct utsname kern_info;
	uname(&kern_info);
	snprintf(str, MAX_STRLEN, "%s %s", kern_info.sysname, kern_info.release);

	return;
}

/*	detect_uptime
	detects the computer's uptime
	argument char *str: the char array to be filled with the uptime in format '$d $h $m $s' where $ is a number
*/
void detect_uptime(char *str)
{
	long uptime = 0;

	#if !defined(__NetBSD__)
		long currtime = 0, boottime = 0;
	#endif

	FILE *uptime_file;

	int secs = 0;
	int mins = 0;
	int hrs = 0;
	int days = 0;

	#if defined(__NetBSD__)
		uptime_file = popen("cut -d ' ' -f 1 < /proc/uptime", "r");
		fscanf(uptime_file, "%ld", &uptime);
		pclose(uptime_file);
	#elif defined(__FreeBSD__) || defined(__DragonFly__)
		uptime_file = popen("sysctl -n kern.boottime | cut -d '=' -f 2 | cut -d ',' -f 1", "r");
		fscanf(uptime_file, "%ld", &boottime); /* get boottime in secs */
		pclose(uptime_file);

		currtime = time(NULL);

		uptime = currtime - boottime;
	#elif defined(__OpenBSD__)
		uptime_file = popen("sysctl -n kern.boottime", "r");
		fscanf(uptime_file, "%ld", &boottime); /* get boottime in secs */
		pclose(uptime_file);

		currtime = time(NULL);

		uptime = currtime - boottime;
	#endif

	split_uptime(uptime, &secs, &mins, &hrs, &days);

	if (days > 0)
		snprintf(str, MAX_STRLEN, "%dd %dh %dm %ds", days, hrs, mins, secs);
	else
		snprintf(str, MAX_STRLEN, "%dh %dm %ds", hrs, mins, secs);

	return;
}

/*	detect_cpu
	detects the computer's CPU brand/name-string
	argument char *str: the char array to be filled with the CPU name
*/
void detect_cpu(char *str)
{
	FILE *cpu_file;

	#if defined(__NetBSD__)
		cpu_file = popen("awk 'BEGIN{FS=\":\"} /model name/ { print $2; exit }' /proc/cpuinfo | sed -e 's/ @/\\n/' -e 's/^ *//g' -e 's/ *$//g' | head -1 | tr -d '\\n'", "r");
		fgets(str, MAX_STRLEN, cpu_file);
		pclose(cpu_file);
	#else
		cpu_file = popen("sysctl -n hw.model | tr -d '\\n'", "r");
		fgets(str, MAX_STRLEN, cpu_file);
		pclose(cpu_file);
	#endif

	return;
}