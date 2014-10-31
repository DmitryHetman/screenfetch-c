/* standard includes */
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <unistd.h>
#include <getopt.h>

/* Windows-specific includes */
#include <Windows.h>
/* Cygwin wets itself without an explicit external linkage to popen */
extern FILE *popen(const char *command, const char *type);
extern int pclose(FILE *stream);

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
	if (STRCMP(str, "Unknown") || STRCMP(str, "*")) /* if distro_str was NOT set by the -D flag or manual mode */
	{
		#if defined(NTDDI_WIN7)
			safe_strncpy(str, "Microsoft Windows 7", MAX_STRLEN);
		#elif defined(NTDDI_WIN8)
			safe_strncpy(str, "Microsoft Windows 8", MAX_STRLEN);
		#elif defined(NTDDI_WINBLUE)
			safe_strncpy(str, "Microsoft Windows 8.1", MAX_STRLEN);
		#elif defined(NTDDI_VISTA) || defined(NTDDI_VISTASP1)
			safe_strncpy(str, "Microsoft Windows Vista", MAX_STRLEN);
		#elif defined(NTDDI_WINXP) || defined(NTDDI_WINXPSP1) || defined(NTDDI_WINXPSP2) || defined(NTDDI_WINXPSP3)
			safe_strncpy(str, "Microsoft Windows XP", MAX_STRLEN);
		#elif defined(_WIN32_WINNT_WS03)
			safe_strncpy(str, "Microsoft Windows Server 2003", MAX_STRLEN);
		#elif defined(_WIN32_WINNT_WS08)
			safe_strncpy(str, "Microsoft Windows Server 2008", MAX_STRLEN);
		#else
			safe_strncpy(str, "Microsoft Windows", MAX_STRLEN);
		#endif
	}

	return;
}