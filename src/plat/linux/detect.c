/* standard includes */
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <unistd.h>
#include <getopt.h>

/* linux-specific includes */
#include <sys/sysinfo.h>
#include <sys/utsname.h>
#include <X11/Xlib.h>
#include <GL/gl.h>
#include <GL/glx.h>
#include <glob.h>

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
		FILE *distro_file;

		char distro_name_str[MAX_STRLEN];

		if (FILE_EXISTS("/system/bin/getprop"))
		{
			safe_strncpy(str, "Android", MAX_STRLEN);
		}
		else
		{
			bool detected = false;

			/* Note: this is a very bad solution, as /etc/issue contains junk on some distros */
			distro_file = fopen("/etc/issue", "r");

			if (distro_file != NULL)
			{
				/* get the first 4 chars, that's all we need */
				fscanf(distro_file, "%4s", distro_name_str);
				fclose(distro_file);

				if (STRCMP(distro_name_str, "Back"))
				{
					safe_strncpy(str, "Backtrack Linux", MAX_STRLEN);
					detected = true;
				}
				else if (STRCMP(distro_name_str, "Crun"))
				{
					safe_strncpy(str, "CrunchBang", MAX_STRLEN);
					detected = true;
				}
				else if (STRCMP(distro_name_str, "LMDE"))
				{
					safe_strncpy(str, "LMDE", MAX_STRLEN);
					detected = true;
				}
				else if (STRCMP(distro_name_str, "Debi") || STRCMP(distro_name_str, "Rasp"))
				{
					safe_strncpy(str, "Debian", MAX_STRLEN);
					detected = true;
				}
			}

			if (!detected)
			{
				if (FILE_EXISTS("/etc/fedora-release"))
				{
					safe_strncpy(str, "Fedora", MAX_STRLEN);
				}
				else if (FILE_EXISTS("/etc/SuSE-release"))
				{
					safe_strncpy(str, "OpenSUSE", MAX_STRLEN);
				}
				else if (FILE_EXISTS("/etc/arch-release"))
				{
					safe_strncpy(str, "Arch Linux", MAX_STRLEN);
				}
				else if (FILE_EXISTS("/etc/gentoo-release"))
				{
					safe_strncpy(str, "Gentoo", MAX_STRLEN);
				}
				else if (FILE_EXISTS("/etc/angstrom-version"))
				{
					safe_strncpy(str, "Angstrom", MAX_STRLEN);
				}
				else if (FILE_EXISTS("/etc/manjaro-release"))
				{
					safe_strncpy(str, "Manjaro", MAX_STRLEN);
				}
				else if (FILE_EXISTS("/etc/lsb-release"))
				{
					distro_file = fopen("/etc/lsb-release", "r");
					fgets(distro_name_str, MAX_STRLEN, distro_file);
					distro_name_str[strlen(distro_name_str) - 1] = '\0';
					fclose(distro_file);

					snprintf(str, MAX_STRLEN, "%s", distro_name_str + 11);
				}
				else
				{
					safe_strncpy(str, "Linux", MAX_STRLEN);

					if (error)
					{
						ERROR_OUT("Error: ", "Failed to detect specific Linux distro.");
					}
				}
			}
		}
	}

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

	int secs = 0;
	int mins = 0;
	int hrs = 0;
	int days = 0;

	struct sysinfo si_upt;
	sysinfo(&si_upt);

	uptime = si_upt.uptime;

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

	cpu_file = popen("awk 'BEGIN{FS=\":\"} /model name/ { print $2; exit }' /proc/cpuinfo | sed -e 's/ @/\\n/' -e 's/^ *//g' -e 's/ *$//g' | head -1 | tr -d '\\n'", "r");
	fgets(str, MAX_STRLEN, cpu_file);
	pclose(cpu_file);

	if (STRCMP(str, "ARMv6-compatible processor rev 7 (v6l)"))
	{
		safe_strncpy(str, "BCM2708 (Raspberry Pi)", MAX_STRLEN); /* quick patch for the Raspberry Pi */
	}

	return;
}

/*	detect_gpu
	detects the computer's GPU brand/name-string
	argument char *str: the char array to be filled with the GPU name
*/
void detect_gpu(char *str, bool error)
{
	Display *disp = NULL;
	Window wind = NULL;
	GLint attr[] = { GLX_RGBA, GLX_DEPTH_SIZE, 24, GLX_DOUBLEBUFFER, None };
	XVisualInfo *visual_info = NULL;
	GLXContext context = NULL;

	if ((disp = XOpenDisplay(NULL)))
	{
		wind = DefaultRootWindow(disp);

		if ((visual_info = glXChooseVisual(disp, 0, attr)))
		{
			if ((context = glXCreateContext(disp, visual_info, NULL, GL_TRUE)))
			{
				glXMakeCurrent(disp, wind, context);
				safe_strncpy(str, (const char *) glGetString(GL_RENDERER), MAX_STRLEN);
			}
			else if (error)
			{
				ERROR_OUT("Error: ", "Failed to create OpenGL context.");
			}
		}
		else if (error)
		{
			ERROR_OUT("Error: ", "Failed to select a proper X visual.");
		}
	}
	else if (error)
	{
		safe_strncpy(str, "No X Server", MAX_STRLEN);
		ERROR_OUT("Error: ", "Could not open an X display.");
	}

	/* cleanup */
	if (context)
		glXDestroyContext(disp, context);
	if (visual_info)
		XFree((void *) visual_info);
	if (disp)
		XCloseDisplay(disp);

	return;
}

/*	detect_disk
	detects the computer's total disk capacity and usage
	argument char *str: the char array to be filled with the disk data in format '$G / $G ($G%)', where $ is a number
*/
void detect_disk(char *str)
{
	FILE *disk_file;

	int disk_total = 0;
	int disk_used = 0;
	int disk_percentage = 0;

	disk_file = popen("df -H 2> /dev/null | grep -vE '^[A-Z]\\:\\/|File' | awk '{ print $2 }' | head -1 | tr -d '\\r\\n G'", "r");
	fscanf(disk_file, "%d", &disk_total);
	pclose(disk_file);

	disk_file = popen("df -H 2> /dev/null | grep -vE '^[A-Z]\\:\\/|File' | awk '{ print $3 }' | head -1 | tr -d '\\r\\n G'", "r");
	fscanf(disk_file, "%d", &disk_used);
	pclose(disk_file);

	if (disk_total > disk_used)
	{
		disk_percentage = (((float) disk_used / disk_total) * 100);

		snprintf(str, MAX_STRLEN, "%dG / %dG (%d%%)", disk_used, disk_total, disk_percentage);
	}
	else /* when disk_used is in a smaller unit */
	{
		disk_percentage = ((float) disk_used / (disk_total * 1024) * 100);

		snprintf(str, MAX_STRLEN, "%dM / %dG (%d%%)", disk_used, disk_total, disk_percentage);
	}

	return;
}

/*	detect_mem
	detects the computer's total and used RAM
	argument char *str: the char array to be filled with the memory data in format '$MB / $MB', where $ is a number
*/
void detect_mem(char *str)
{
	long long total_mem = 0; /* each of the following MAY contain bytes/kbytes/mbytes/pages */
	long long free_mem = 0;
	long long used_mem = 0;

	/* known problem: because linux utilizes free ram extensively in caches/buffers,
	   the amount of memory sysinfo reports as free is very small.
	*/
	struct sysinfo si_mem;
	sysinfo(&si_mem);

	total_mem = (long long) (si_mem.totalram * si_mem.mem_unit) / MB;
	free_mem = (long long) (si_mem.freeram * si_mem.mem_unit) / MB;
	used_mem = (long long) total_mem - free_mem;

	snprintf(str, MAX_STRLEN, "%lld%s / %lld%s", used_mem, "MB", total_mem, "MB");

	return;
}

/*	detect_shell
	detects the shell currently running on the computer
	argument char *str: the char array to be filled with the shell name and version
	--
	CAVEAT: shell version detection relies on the standard versioning format for 
	each shell. If any shell's older (or newer versions) suddenly begin to use a new
	scheme, the version may be displayed incorrectly.
	--
*/
void detect_shell(char *str, bool error)
{
	FILE *shell_file;

	char *shell_name;
	char vers_str[MAX_STRLEN];

	shell_name = getenv("SHELL");

	if (shell_name == NULL)
	{
		if (error)
			ERROR_OUT("Error: ", "Problem detecting shell.");

		return;
	}

	if (strstr(shell_name, "bash"))
	{
		shell_file = popen("bash --version | head -1", "r");
		fgets(vers_str, MAX_STRLEN, shell_file);
		/* evil pointer arithmetic */
		snprintf(str, MAX_STRLEN, "bash %.*s", 17, vers_str + 10);
		pclose(shell_file);
	}

	else if (strstr(shell_name, "zsh"))
	{
		shell_file = popen("zsh --version", "r");
		fgets(vers_str, MAX_STRLEN, shell_file);	
		/* evil pointer arithmetic */
		snprintf(str, MAX_STRLEN, "zsh %.*s", 5, vers_str + 4);
		pclose(shell_file);
	}

	else if (strstr(shell_name, "csh"))
	{
		shell_file = popen("csh --version | head -1", "r");
		fgets(vers_str, MAX_STRLEN, shell_file);
		/* evil pointer arithmetic */
		snprintf(str, MAX_STRLEN, "csh %.*s", 7, vers_str + 5);
		pclose(shell_file);
	}

	else if (strstr(shell_name, "fish"))
	{
		shell_file = popen("fish --version", "r");
		fgets(vers_str, MAX_STRLEN, shell_file);
		/* evil pointer arithmetic */
		snprintf(str, MAX_STRLEN, "fish %.*s", 13, vers_str + 6);
		pclose(shell_file);
	}

	else if (strstr(shell_name, "dash") || strstr(shell_name, "ash") || strstr(shell_name, "ksh"))
	{
		/* i don't have a version detection system for these, yet */
		safe_strncpy(str, shell_name, MAX_STRLEN);
	}

	return;
}

/*	detect_res
	detects the combined resolution of all monitors attached to the computer
	argument char *str: the char array to be filled with the resolution in format '$x$', where $ is a number
*/
void detect_res(char *str, bool error)
{
	int width = 0;
	int height = 0;

	Display *disp;

	if ((disp = XOpenDisplay(NULL)))
	{
		Screen *screen = XDefaultScreenOfDisplay(disp);
		width = WidthOfScreen(screen);
		height = HeightOfScreen(screen);
		snprintf(str, MAX_STRLEN, "%dx%d", width, height);
	}
	else
	{
		safe_strncpy(str, "No X Server", MAX_STRLEN);

		if (error)
			ERROR_OUT("Error: ", "Problem detecting X display resolution.");
	}

	XCloseDisplay(disp);

	return;
}