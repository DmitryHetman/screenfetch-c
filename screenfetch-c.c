/* 	screenfetch-c.c
	Author: William Woodruff
	-------------

	A rewrite of screenFetch.sh 3.0.5 in C.
	This is primarily an experiment borne out of an awareness of the slow execution time on the 
	screenfetch-dev.sh script. 
	Hopefully this port will execute faster, although it's more for self education than anything else.

	NOTES:
	I used many of Brett Bohnenkamper's awk/sed/grep/etc oneliners in my popen() calls, 
	although some were modified to change/improve the output.
	Credit goes to shrx and Hu6 for many of the oneliners used in screenfetch-c's OS X popen() calls.

	PLANNED IMPROVEMENTS:
	Add libcpuid to decrease reliance on shell utilities.
	Streamline code.

	TODO:
	Figure out DE/WM/WM theme/GTK/Android detection

	**From the original author:**

	--

	Copyright (c) 2010-2012 Brett Bohnenkamper < kittykatt AT archlinux DOT us >

	Permission is hereby granted, free of charge, to any person obtaining a copy of this software
	and associated documentation files (the "Software"), to deal in the Software without restriction,
	including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense,
	and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so,
	subject to the following conditions:

	The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

	THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
	WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR
	COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE,
	ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

	Yes, I do realize some of this is horribly ugly coding. Any ideas/suggestions would be
	appreciated by emailing me or by stopping by http://github.com/KittyKatt/screenFetch . You
	could also drop in on my IRC network, SilverIRC, at irc://kittykatt.silverirc.com:6667/screenFetch
	to put forth suggestions/ideas. Thank you.

	--

	I hereby regrant this version of screenFetch under the same MIT license.
	If you have any questions, please contact me at woodrufw@bxscience.edu or on github (http://www.github.com/woodrufw/screenfetch-c)
*/
#define _XOPEN_SOURCE 700

#include <stdio.h> //for a medley of I/O operations, including popen/pclose
#include <stdlib.h> //for getenv, etc
#include <stdbool.h> //for the bool type
#include <string.h> //for strcmp, strncpy, etc.
#include <unistd.h> //for sleep, getopt
#include "screenfetch-c.h" //contains function prototypes, macros, ascii logos

//string definitions
static char distro_str[MAX_STRLEN];
static char arch_str[MAX_STRLEN];
static char host_str[MAX_STRLEN];
static char kernel_str[MAX_STRLEN];
static char uptime_str[MAX_STRLEN];
static char pkgs_str[MAX_STRLEN];
static char cpu_str[MAX_STRLEN];
static char gpu_str[MAX_STRLEN];
static char disk_str[MAX_STRLEN];
static char mem_str[MAX_STRLEN];
static char shell_str[MAX_STRLEN];
static char shell_version_str[MAX_STRLEN];
static char res_str[MAX_STRLEN];
static char de_str[MAX_STRLEN];
static char wm_str[MAX_STRLEN];
static char wm_theme_str[MAX_STRLEN];
static char gtk_str[MAX_STRLEN];
static char* detected_arr[16];
static char* detected_arr_names[16] = {"", "OS: ", "Kernel: ", "Arch: ", "CPU: ", "GPU: ", "Shell: ", "Packages: ", "Disk: ", "Memory: ", "Uptime: ", "Resolution: ", "DE: ", "WM: ", "WM Theme: ", "GTK: "};

//other definitions
bool debug = false;
bool error = true;
bool verbose = false;
bool screenshot = false;
bool ascii = false;

int main(int argc, char** argv)
{
	//first off, don't allow unknown OSes to run this program
	if (OS == UNKNOWN)
	{
		ERROR_OUT("Error: ", "This program isn't designed for your OS.");
		ERROR_OUT("Even if it did compile successfully, it will not execute correctly.", "");

		return EXIT_FAILURE;
	}

	//copy 'Unknown' to each string and append a null character
	safe_strncpy(distro_str, "Unknown", MAX_STRLEN);
	safe_strncpy(arch_str, "Unknown", MAX_STRLEN);
	safe_strncpy(host_str, "Unknown", MAX_STRLEN);
	safe_strncpy(kernel_str, "Unknown", MAX_STRLEN);
	safe_strncpy(uptime_str, "Unknown", MAX_STRLEN);
	safe_strncpy(pkgs_str, "Unknown", MAX_STRLEN);
	safe_strncpy(cpu_str, "Unknown", MAX_STRLEN);
	safe_strncpy(gpu_str, "Unknown", MAX_STRLEN);
	safe_strncpy(disk_str, "Unknown", MAX_STRLEN);
	safe_strncpy(mem_str, "Unknown", MAX_STRLEN);
	safe_strncpy(shell_str, "Unknown", MAX_STRLEN);
	safe_strncpy(shell_version_str, "Unknown", MAX_STRLEN);
	safe_strncpy(res_str, "Unknown", MAX_STRLEN);
	safe_strncpy(de_str, "Unknown", MAX_STRLEN);
	safe_strncpy(wm_str, "Unknown", MAX_STRLEN);
	safe_strncpy(wm_theme_str, "Unknown", MAX_STRLEN);
	safe_strncpy(gtk_str, "Unknown", MAX_STRLEN);

	char c; 

	while ((c = getopt(argc, argv, "dvnNsS:D:A:EVh")) != -1)
	{
		switch (c)
		{
			case 'd':
				SET_DEBUG(true);
				break;
			case 'v':
				SET_VERBOSE(true);
				break;
			case 'n':
				//something like SET_LOGO(false);
				break;
			case 'N':
				//something like SET_COLOR(false);
				break;
			case 's':
				SET_SCREENSHOT(true);
				break;
			case 'S':
				SET_SCREENSHOT(true);
				//do something with optarg
				break;
			case 'D':
				SET_DISTRO(optarg);
				break;
			case 'A':
				//something like SET_DISTRO_ART(optarg);
				break;
			case 'E':
				SET_ERROR(false);
				break;
			case 'V':
				display_version();
				return EXIT_SUCCESS;
			case 'h':
				display_help();
				return EXIT_SUCCESS;
			case '?':
				if (optopt == 'S' || optopt == 'D' || optopt == 'A')
					ERROR_OUT("Error: ", "One or more tripped flag(s) requires an argument.");
				else
					ERROR_OUT("Error: ", "Unknown option or option character.");
				return EXIT_FAILURE;
		}
	}

	//each string is filled by its respective function
	detect_distro(distro_str);
	detect_arch(arch_str);
	detect_host(host_str);
	detect_kernel(kernel_str);
	detect_uptime(uptime_str);
	detect_pkgs(pkgs_str);
	detect_cpu(cpu_str);
	detect_gpu(gpu_str);
	detect_disk(disk_str);
	detect_mem(mem_str);
	detect_shell(shell_str);
	detect_shell_version(shell_version_str);
	detect_res(res_str);
	detect_de(de_str);
	detect_wm(wm_str);
	detect_wm_theme(wm_theme_str);
	detect_gtk(gtk_str);

	//detected_arr is filled with the gathered from the detection functions
	fill_detected_arr(detected_arr, distro_str, arch_str, host_str, kernel_str, uptime_str, pkgs_str, cpu_str, gpu_str, disk_str, mem_str, shell_str, shell_version_str, res_str, de_str, wm_str, wm_theme_str, gtk_str);

	//actual output
	main_output();

	if (screenshot)
	{
		take_screenshot();
	}

	//debug section - only executed if -d flag is tripped
	if (debug)
	{
		printf("%s\n%s\n%s\n%s\n%s\n%s\n%s\n%s\n%s\n%s\n%s\n%s\n%s\n%s\n%s\n%s\n", distro_str, arch_str, host_str, kernel_str, uptime_str, pkgs_str, cpu_str, gpu_str, disk_str, mem_str, shell_str, shell_version_str, res_str, de_str, wm_str, wm_theme_str, gtk_str);

		if (STRCMP(distro_str, "Unknown"))
		{
			DEBUG_OUT("Distro detection failure: ", distro_str);
		}

		if (STRCMP(arch_str, "Unknown"))
		{
			DEBUG_OUT("Architecture detection failure: ", arch_str);
		}

		if (STRCMP(host_str, "Unknown"))
		{
			DEBUG_OUT("Host detection failure: ", host_str);
		}

		if (STRCMP(kernel_str, "Unknown"))
		{
			DEBUG_OUT("Kernel detection failure: ", kernel_str);
		}

		if (STRCMP(uptime_str, "Unknown"))
		{
			DEBUG_OUT("Uptime detection failure: ", uptime_str);
		}

		if (STRCMP(pkgs_str, "Unknown"))
		{
			DEBUG_OUT("Package detection failure: ", pkgs_str);
		}

		if (STRCMP(cpu_str, "Unknown"))
		{
			DEBUG_OUT("CPU detection failure: ", cpu_str);
		}

		if (STRCMP(gpu_str, "Unknown"))
		{
			DEBUG_OUT("GPU detection failure: ", gpu_str);
		}

		if (STRCMP(disk_str, "Unknown"))
		{
			DEBUG_OUT("Disk detection failure: ", disk_str);
		}

		if (STRCMP(mem_str, "Unknown"))
		{
			DEBUG_OUT("Memory detection failure: ", mem_str);
		}

		if (STRCMP(shell_str, "Unknown"))
		{
			DEBUG_OUT("Shell detection failure: ", shell_str);
		}

		if (STRCMP(shell_version_str, "Unknown"))
		{
			DEBUG_OUT("Shell version detection failure: ", shell_version_str);
		}

		if (STRCMP(res_str, "Unknown"))
		{
			DEBUG_OUT("Resolution detection failure: ", res_str);
		}

		if (STRCMP(de_str, "Unknown"))
		{
			DEBUG_OUT("DE detection failure: ", de_str);
		}

		if (STRCMP(wm_str, "Unknown"))
		{
			DEBUG_OUT("WM detection failure: ", wm_str);
		}

		if (STRCMP(wm_theme_str, "Unknown"))
		{
			DEBUG_OUT("WM Theme detection failure: ", wm_theme_str);
		}

		if (STRCMP(gtk_str, "Unknown"))
		{
			DEBUG_OUT("GTK detection failure: ", gtk_str);
		}
	}

	return EXIT_SUCCESS;
}

/*  **  BEGIN DETECTION FUNCTIONS  ** */

//detect_distro
//detects the computer's distribution (really only relevant on Linux)
//returns a string containing the distro name (may vary in format)
void detect_distro(char* str)
{
	if (STRCMP(str, "Unknown")) //if distro_str was NOT set by the -D flag
	{
		FILE* distro_file;

		char distro_name_str[MAX_STRLEN];

		if (OS == CYGWIN)
		{
			distro_file = popen("wmic os get name | head -2 | tail -1", "r");
			fgets(distro_name_str, MAX_STRLEN, distro_file);
			pclose(distro_file);

			//currently only works on W7, working on a solution
			snprintf(str, MAX_STRLEN, "%.*s", 19, distro_name_str);
		}

		else if (OS == OSX)
		{
			distro_file = popen("sw_vers | grep ProductVersion | tr -d 'ProductVersion: \\t\\n'", "r");
			fgets(distro_name_str, MAX_STRLEN, distro_file);
			pclose(distro_file);

			snprintf(str, MAX_STRLEN, "%s%s", "Mac OS X ", distro_name_str);
		}

		else if (OS == LINUX)
		{
			distro_file = fopen("/etc/lsb-release", "r");

			if (distro_file != NULL)
			{
				fclose(distro_file);

				distro_file = popen("cat /etc/lsb-release | head -1 | tr -d \"DISTRIB_ID=\\n\"", "r");
				fgets(str, MAX_STRLEN, distro_file);
				pclose(distro_file);
			}

			else //begin the tedious task of checking each /etc/*-release
			{
				distro_file = fopen("/etc/fedora-release", "r");

				if (distro_file != NULL)
				{
					fclose(distro_file);
					safe_strncpy(str, "Fedora", MAX_STRLEN);
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

		else if (ISBSD())
		{
			distro_file = popen("uname -sr", "r");
			fgets(str, sizeof(str), distro_file);
			pclose(distro_file);
		}
	}

	if (verbose)
	{
		VERBOSE_OUT("Found distro as ", str);
	}

	return;
}

//detect_arch
//detects the computer's architecture
//returns a string containing the arch
void detect_arch(char* str)
{
	FILE* arch_file;

	if (OS == CYGWIN)
	{
		arch_file = popen("wmic os get OSArchitecture | head -2 | tail -1 | tr -d '\\r\\n '", "r");
		fgets(str, MAX_STRLEN, arch_file);
		pclose(arch_file);
	}

	else if (OS == OSX || OS == LINUX || ISBSD())
	{
		arch_file = popen("uname -m | tr -d '\\n'", "r");
		fgets(str, MAX_STRLEN, arch_file);
		pclose(arch_file);
	}

	if (verbose)
	{
		VERBOSE_OUT("Found system arch as ", str);
	}

	return;
}

//detect_host
//detects the computer's hostname and active user and formats them
//returns a string of format "user@hostname"
void detect_host(char* str)
{
	char* given_user; //has to be a pointer for getenv()
	char given_host[MAX_STRLEN];

	given_user = getenv("USER");

	FILE* host_file = popen("hostname | tr -d '\\r\\n '", "r");
	fgets(given_host, MAX_STRLEN, host_file);
	pclose(host_file);

	//format str
	snprintf(str, MAX_STRLEN, "%s@%s", given_user, given_host);

	if (verbose)
	{
		VERBOSE_OUT("Found host as ", str);
	}

	return;
}

//detect_kernel
//detects the computer's kernel
//returns a string containing the kernel name, version, etc
void detect_kernel(char* str)
{
	FILE* kernel_file = popen("uname -sr | tr -d '\\r\\n'", "r");
	fgets(str, MAX_STRLEN, kernel_file);
	pclose(kernel_file);

	if (verbose)
	{
		VERBOSE_OUT("Found kenel as ", str);
	}

	return;
}

//detect_uptime
//detects the computer's uptime
//returns a string of format "<>d <>h <>m <>s", where <> is a number
void detect_uptime(char* str)
{
	FILE* uptime_file;

	long uptime;
	long now, boottime; //may or may not be used depending on OS
	int secs;
	int mins;
	int hrs;
	int days;

	if (OS == CYGWIN || OS == NETBSD)
	{
		uptime_file = popen("cat /proc/uptime | cut -d ' ' -f 1", "r");
		fscanf(uptime_file, "%ld", &uptime);
		pclose(uptime_file);
	}

	else if (OS == OSX || OS == FREEBSD || OS == DFBSD)
	{
		uptime_file = popen("sysctl -n kern.boottime | cut -d \"=\" -f 2 | cut -d \",\" -f 1", "r");
		fscanf(uptime_file, "%ld", &boottime); //get boottime in secs
		pclose(uptime_file);

		uptime_file = popen("date +%s", "r");
		fscanf(uptime_file, "%ld", &now); //get current time in secs
		pclose(uptime_file);

		uptime = now - boottime;
	}

	else if (OS == LINUX)
	{
		#ifdef __linux__
			struct sysinfo si_upt;
			sysinfo(&si_upt);

			uptime = si_upt.uptime;
		#endif
	}

	else if (OS == OPENBSD)
	{
		uptime_file = popen("sysctl -n kern.boottime", "r");
		fscanf(uptime_file, "%ld", &boottime); //get current boottime in secs
		pclose(uptime_file);

		uptime_file = popen("date +%s", "r");
		fscanf(uptime_file, "%ld", &now); //get current time in secs
		pclose(uptime_file);

		uptime = now - boottime;
	}

	split_uptime(uptime, &secs, &mins, &hrs, &days);

	if (days > 0)
		snprintf(str, MAX_STRLEN, "%dd %dh %dm %ds", days, hrs, mins, secs);
	else
		snprintf(str, MAX_STRLEN, "%dh %dm %ds", hrs, mins, secs);

	if (verbose)
	{
		VERBOSE_OUT("Found uptime as ", str);
	}

	return;
}

//detect_pkgs
//detects the number of packages installed on the computer
//returns a string containing the number of packages
void detect_pkgs(char* str)
{
	FILE* pkgs_file;

	int packages = 0;

	if (OS == CYGWIN)
	{
		pkgs_file = popen("cygcheck -cd | wc -l", "r");
		fscanf(pkgs_file, "%d", &packages);
		packages -= 2;
		pclose(pkgs_file);

		snprintf(str, MAX_STRLEN, "%d", packages);
	}

	else if (OS == OSX)
	{
		pkgs_file = popen("ls /usr/local/bin | wc -w", "r");
		fscanf(pkgs_file, "%d", &packages);
		pclose(pkgs_file);

		pkgs_file = fopen("/usr/local/bin/brew", "r"); //test for existence of homebrew

		if (pkgs_file != NULL)
		{
			fclose(pkgs_file);

			int brew_pkgs = 0;
			pkgs_file = popen("brew list -1 | wc -l", "r");
			fscanf(pkgs_file, "%d", &brew_pkgs);
			pclose(pkgs_file);

			packages += brew_pkgs; 
		}

		//test for existence of macports, fink, etc here
	}

	else if (OS == LINUX)
	{
		if (STRCMP(distro_str, "Arch Linux") || STRCMP(distro_str, "ParabolaGNU/Linux-libre") || STRCMP(distro_str, "Chakra") || STRCMP(distro_str, "Manjaro"))
		{
			pkgs_file = popen("pacman -Qq | wc -l", "r");
			fscanf(pkgs_file, "%d", &packages);
			pclose(pkgs_file);
		}

		else if (STRCMP(distro_str, "Frugalware"))
		{
			pkgs_file = popen("pacman-g2 -Q | wc -l", "r");
			fscanf(pkgs_file, "%d", &packages);
			pclose(pkgs_file);
		}

		else if (STRCMP(distro_str, "Fuduntu") || STRCMP(distro_str, "Ubuntu") || STRCMP(distro_str, "LinuxMint") || STRCMP(distro_str, "SolusOS") || STRCMP(distro_str, "Debian") || STRCMP(distro_str, "LMDE") || STRCMP(distro_str, "CrunchBang") || STRCMP(distro_str, "Peppermint") || STRCMP(distro_str, "LinuxDeepin") || STRCMP(distro_str, "Trisquel") || STRCMP(distro_str, "Elementary OS"))
		{
			pkgs_file = popen("dpkg --get-selections | wc -l", "r");
			fscanf(pkgs_file, "%d", &packages);
			pclose(pkgs_file);
		}

		else if (STRCMP(distro_str, "Slackware"))
		{
			pkgs_file = popen("ls -l /var/log/packages | wc -l", "r");
			fscanf(pkgs_file, "%d", &packages);
			pclose(pkgs_file);
		}

		else if (STRCMP(distro_str, "Gentoo") || STRCMP(distro_str, "Sabayon") || STRCMP(distro_str, "Funtoo"))
		{
			pkgs_file = popen("ls -d /var/db/pkg/*/* | wc -l", "r");
			fscanf(pkgs_file, "%d", &packages);
			pclose(pkgs_file);
		}

		else if (STRCMP(distro_str, "Fedora") || STRCMP(distro_str, "openSUSE") || STRCMP(distro_str, "Red Hat Linux") || STRCMP(distro_str, "Mandriva") || STRCMP(distro_str, "Mandrake") || STRCMP(distro_str, "Mageia") || STRCMP(distro_str, "Viperr"))
		{
			pkgs_file = popen("rpm -qa | wc -l", "r");
			fscanf(pkgs_file, "%d", &packages);
			pclose(pkgs_file);
		}

		//if linux disto detection failed
		else if (STRCMP(distro_str, "Linux") && error)
		{
			safe_strncpy(str, "Not Found", MAX_STRLEN);
			ERROR_OUT("Error: ", "Packages cannot be detected on an unknown Linux distro.");
		}
	}

	else if (OS == FREEBSD || OS == OPENBSD)
	{
		pkgs_file = popen("pkg_info | wc -l | awk '{sub(\" \", \"\");print $1}'", "r");

		pclose(pkgs_file);
	}

	else if (OS == NETBSD || OS == DFBSD)
	{
		safe_strncpy(str, "Not Found", MAX_STRLEN);
		ERROR_OUT("Error: ", "Could not find packages on current OS.");
	}

	if (verbose)
	{
		VERBOSE_OUT("Found package count as ", str);
	}

	snprintf(str, MAX_STRLEN, "%d", packages);

	return;
}

//detect_cpu
//detects the computer's CPU brand/name-string
//returns a string containing the CPU string
void detect_cpu(char* str)
{
	FILE* cpu_file;

	if (OS == CYGWIN)
	{
		cpu_file = popen("wmic cpu get name | tail -2 | tr -d '\\r\\n'", "r");
		fgets(str, MAX_STRLEN, cpu_file);
		pclose(cpu_file);
	}

	else if (OS == OSX)
	{
		cpu_file = popen("sysctl -n machdep.cpu.brand_string | tr -d '\\n'", "r");
		fgets(str, MAX_STRLEN, cpu_file);
		pclose(cpu_file);
	}

	else if (OS == LINUX || OS == NETBSD)
	{
		cpu_file = popen("awk 'BEGIN{FS=\":\"} /model name/ { print $2; exit }' /proc/cpuinfo | sed 's/ @/\\n/' | head -1 | tr -d \"\\n\"", "r");
		fgets(str, MAX_STRLEN, cpu_file);
		pclose(cpu_file);
	}

	else if (OS == DFBSD || OS == FREEBSD || OS == OPENBSD)
	{
		cpu_file = popen("sysctl -n hw.model", "r");
		fgets(str, MAX_STRLEN, cpu_file);
		pclose(cpu_file);
	}

	if (verbose)
	{
		VERBOSE_OUT("Found CPU as ", str);
	}

	return;
}

//detect_gpu
//detects the computer's GPU brand/name-string
//returns a string containing the GPU string
void detect_gpu(char* str)
{
	FILE* gpu_file;

	if (OS == CYGWIN)
	{
		gpu_file = popen("wmic path Win32_VideoController get caption | tail -2 | tr -d '\\r\\n'", "r");
		fgets(str, MAX_STRLEN, gpu_file);
		pclose(gpu_file);
	}

	else if (OS == OSX)
	{
		gpu_file = popen("system_profiler SPDisplaysDataType | awk -F': ' '/^\\ *Chipset Model:/ {print $2}' | tr -d '\\n'", "r");
		fgets(str, MAX_STRLEN, gpu_file);
		pclose(gpu_file);
	}

	else if (OS == LINUX || ISBSD())
	{

	}

	if (verbose)
	{
		VERBOSE_OUT("Found GPU as ", str);
	}

	return;
}

//detect_disk
//detects the computer's total HDD/SSD capacity and usage
//returns a string of format: "<>G / <>G", where <> is a number
void detect_disk(char* str)
{
	FILE* disk_file;

	char disk_total_str[MAX_STRLEN];
	char disk_free_str[MAX_STRLEN];

	if (OS == CYGWIN || OS == LINUX || OS == OSX)
	{
		disk_file = popen("df -H | grep -vE '^[A-Z]\\:\\/|File' | awk '{ print $2 }' | head -1 | tr -d '\\r\\n '", "r");
		fgets(disk_total_str, MAX_STRLEN, disk_file);
		pclose(disk_file);

		disk_file = popen("df -H | grep -vE '^[A-Z]\\:\\/|File' | awk '{ print $4 }' | head -1 | tr -d '\\r\\n '", "r");
		fgets(disk_free_str, MAX_STRLEN, disk_file);
		pclose(disk_file);
	}

	else if (ISBSD())
	{
		disk_file = popen("df -h | grep -vE '^[A-Z]\\:\\/|File' | awk '{ print $2 }' | head -1 | tr -d '\\r '", "r");
		fgets(disk_total_str, MAX_STRLEN, disk_file);
		pclose(disk_file);

		disk_file = popen("df -h | grep -vE '^[A-Z]\\:\\/|File' | awk '{ print $4 }' | head -1 | tr -d '\\r '", "r");
		fgets(disk_free_str, MAX_STRLEN, disk_file);
		pclose(disk_file);
	}

	snprintf(str, MAX_STRLEN, "%s / %s", disk_free_str, disk_total_str);

	if (verbose)
	{
		VERBOSE_OUT("Found disk usage as ", str);
	}

	return;
}

//detect_mem
//detects the computer's total and used RAM
//returns a string of format: "<>MB / <>MB", where <> is a number
void detect_mem(char* str)
{
	FILE* mem_file;

	long kb = 1024;
	long mb = kb * kb;
	long total_mem; // each of the following MAY contain bytes/kbytes/mbytes/pages
	long free_mem; 
	long used_mem;

	if (OS == CYGWIN || OS == NETBSD)
	{
		mem_file = popen("awk '/MemTotal/ { print $2 }' /proc/meminfo", "r");
		fscanf(mem_file, "%ld", &total_mem);
		pclose(mem_file);

		mem_file = popen("awk '/MemFree/ { print $2 }' /proc/meminfo", "r");
		fscanf(mem_file, "%ld", &free_mem);
		pclose(mem_file);

		total_mem /= (long) kb;
		free_mem /= (long) kb;
		used_mem = total_mem - free_mem;
	}

	else if (OS == OSX)
	{
		mem_file = popen("sysctl -n hw.memsize", "r");
		fscanf(mem_file, "%ld", &total_mem);
		pclose(mem_file);

		mem_file = popen("vm_stat | head -2 | tail -1 | tr -d \"Pages free: .\"", "r");
		fscanf(mem_file, "%ld", &free_mem);
		pclose(mem_file);

		total_mem /= (long) mb;

		free_mem *= 4096; //4KiB is OS X's page size
		free_mem /= (long) mb;

		used_mem = total_mem - free_mem;
	}

	else if (OS == LINUX)
	{
		#ifdef __linux__
		struct sysinfo si_mem;
		sysinfo(&si_mem);

		total_mem = (long) si_mem.totalram / mb;
		free_mem = (long) si_mem.freeram / mb;
		used_mem = (long) total_mem - free_mem;
		#endif
	}

	else if (OS == FREEBSD)
	{
		//it's unknown if FreeBSD's /proc/meminfo is in the same format as Cygwin's

		mem_file = popen("awk '/MemTotal/ { print $2 }' /proc/meminfo", "r");
		fscanf(mem_file, "%ld", &total_mem);
		pclose(mem_file);

		mem_file = popen("awk '/MemFree/ { print $2 }' /proc/meminfo", "r");
		fscanf(mem_file, "%ld", &free_mem);
		pclose(mem_file);

		total_mem /= (long) kb;
		free_mem /= (long) kb;
		used_mem = total_mem - free_mem;
	}

	else if (OS == OPENBSD)
	{
		mem_file = popen("top -1 1 | awk '/Real:/ {k=split($3,a,\"/\");print a[k] }' | tr -d 'M'", "r");
		fscanf(mem_file, "%ld", &total_mem);
		pclose(mem_file);

		mem_file = popen("top -1 1 | awk '/Real:/ {print $3}' | sed 's/M.*//'", "r");
		fscanf(mem_file, "%ld", &used_mem);
		pclose(mem_file);
	}

	else if (OS == DFBSD)
	{

	}

	snprintf(str, MAX_STRLEN, "%ld%s / %ld%s", used_mem, "MB", total_mem, "MB");

	if (verbose)
	{
		VERBOSE_OUT("Found memory usage as ", str);
	}

	return;
}

//detect_shell
//detects the shell currently running on the computer
//returns a string containing the name of that shell
void detect_shell(char* str)
{
	FILE* shell_file;

	shell_file = popen("echo $SHELL | awk -F \"/\" '{print $NF}' | tr -d '\\r\\n'", "r");
	fgets(str, 128, shell_file);
	pclose(shell_file);

	return;
}

//detect_shell_version
//detects the version of the shell detected in detect_shell()
//returns a string containing the version
void detect_shell_version(char* str)
{
	FILE* shell_version_file;

	char temp_vers_str[MAX_STRLEN];

	if (STRCMP(shell_str, "bash"))
	{
		shell_version_file = popen("bash --version | head -1", "r");
		fgets(temp_vers_str, MAX_STRLEN, shell_version_file);
		//evil pointer arithmetic
		snprintf(str, MAX_STRLEN, "%.*s", 17, temp_vers_str + 10);
		pclose(shell_version_file);
	}

	else if (STRCMP(shell_str, "zsh"))
	{
		shell_version_file = popen("zsh --version", "r");
		fgets(temp_vers_str, MAX_STRLEN, shell_version_file);	
		//evil pointer arithmetic
		snprintf(str, MAX_STRLEN, "%.*s", 5, temp_vers_str + 4);
		pclose(shell_version_file);
	}

	else if (STRCMP(shell_str, "csh"))
	{
		shell_version_file = popen("csh --version | head -1", "r");
		fgets(temp_vers_str, MAX_STRLEN, shell_version_file);
		//evil pointer arithmetic
		snprintf(str, MAX_STRLEN, "%.*s", 7, temp_vers_str + 5);
		pclose(shell_version_file);
	}

	else if (STRCMP(shell_str, "ksh"))
	{
		
	}
	
	else if (STRCMP(shell_str, "fish"))
	{
		shell_version_file = popen("fish --version", "r");
		fgets(temp_vers_str, MAX_STRLEN, shell_version_file);
		//evil pointer arithmetic
		snprintf(str, MAX_STRLEN, "%.*s", 13, str + 6);
		pclose(shell_version_file);
	}

	else if (STRCMP(shell_str, "dash"))
	{

	}
	
	if (verbose)
	{
		VERBOSE_OUT("Found shell version as ", str);
	}

	return;
}

//detect_res
//detects the combined resoloution of all monitors attached to the computer
//returns a string of format: "<>x<>", where <> is a number
void detect_res(char* str)
{
	FILE* res_file;

	char width_str[MAX_STRLEN];
	char height_str[MAX_STRLEN];

	if (OS == CYGWIN)
	{
		res_file = popen("wmic desktopmonitor get screenwidth | tail -2 | tr -d '\\r\\n '", "r");
		fgets(width_str, MAX_STRLEN, res_file);
		pclose(res_file);

		res_file = popen("wmic desktopmonitor get screenheight | tail -2 | tr -d '\\r\\n '", "r");
		fgets(height_str, MAX_STRLEN, res_file);
		pclose(res_file);

		snprintf(str, MAX_STRLEN, "%sx%s", width_str, height_str);
	}

	else if (OS == OSX)
	{
		res_file = popen("system_profiler SPDisplaysDataType | awk '/Resolution:/ {print $2\"x\"$4\" \"}' | tr -d '\\n'", "r");
		fgets(str, MAX_STRLEN, res_file);
		pclose(res_file);
	}

	else if (OS == LINUX)
	{
		res_file = popen("xdpyinfo | sed -n 's/.*dim.* \\([0-9]*x[0-9]*\\) .*/\\1/pg' | sed ':a;N;$!ba;s/\\n/ /g' | tr -d '\\n'", "r");
		fgets(str, MAX_STRLEN, res_file);
		pclose(res_file);
	}

	else if (ISBSD())
	{
		res_file = popen("xdpyinfo | sed -n 's/.*dim.* \\([0-9]*x[0-9]*\\) .*/\\1/pg' | tr '\\n' ' '", "r");
		fgets(str, MAX_STRLEN, res_file);
		pclose(res_file);
	}

	if (verbose)
	{
		VERBOSE_OUT("Found resolution as ", str);
	}

	return;
}

//detect_de
//detects the desktop environment currently running on top of the OS
//returns a string containing the name of the DE
void detect_de(char* str)
{
	FILE* de_file;

	if (OS == CYGWIN)
	{
		int version;

		de_file = popen("wmic os get version | grep -o '^[0-9]'", "r");
		fscanf(de_file, "%d", &version);
		pclose(de_file);

		if (version == 6 || version == 7)
		{
			safe_strncpy(str, "Aero", MAX_STRLEN);
		}
		else
		{
			safe_strncpy(str, "Luna", MAX_STRLEN);
		}
	}

	else if (OS == OSX)
	{
		safe_strncpy(str, "Aqua", MAX_STRLEN);
	}

	else if (OS == LINUX || ISBSD())
	{
		//this is going to be complicated
	}

	if (verbose)
	{
		VERBOSE_OUT("Found DE as ", str);
	}

	return;
}

//detect_wm
//detects the window manager currently running on top of the OS
//returns a string containing the name of the WM
void detect_wm(char* str)
{
	FILE* wm_file;

	char test_str[MAX_STRLEN];

	if (OS == CYGWIN)
	{
		wm_file = popen("tasklist | grep -o 'bugn' | tr -d '\\r\\n'", "r");
		//test for bugn
		pclose(wm_file);

		wm_file = popen("tasklist | grep -o 'Windawesome' | tr -d '\\r \\n'", "r");
		//test for Windawesome
		pclose(wm_file);

		//else
		safe_strncpy(str, "DWM", MAX_STRLEN);
	}

	else if (OS == OSX)
	{
		safe_strncpy(str, "Quartz Compositor", MAX_STRLEN);
	}

	else if (OS == LINUX || ISBSD())
	{

	}

	if (verbose)
	{
		VERBOSE_OUT("Found WM as ", str);
	}

	return;
}

//detect_wm_theme
//detects the theme associated with the WM detected in detect_wm()
//returns a string containing the name of the WM theme
void detect_wm_theme(char* str)
{
	FILE* wm_theme_file;

	if (OS == CYGWIN)
	{
		//nasty one-liner
		wm_theme_file = popen("reg query 'HKCU\\Software\\Microsoft\\Windows\\CurrentVersion\\Themes' /v 'CurrentTheme' | grep -o '[A-Z]:\\\\.*' | awk -F\"\\\\\" '{print $NF}' | grep -o '[0-9A-z. ]*$' | grep -o '^[0-9A-z ]*' | tr -d '\\r\\n'", "r");
		fgets(str, MAX_STRLEN, wm_theme_file);
		pclose(wm_theme_file);
	}


	if (verbose)
	{
		VERBOSE_OUT("Found WM theme as ", str);
	}

	return;
}

//detect_gtk
//detects the theme, icon(s), and font(s) associated with a GTK DE (if present)
//returns a string containing the name of the gtk theme
void detect_gtk(char* str)
{
	FILE* gtk_file;

	char font_str[MAX_STRLEN];

	if (OS == CYGWIN)
	{
		//get the terminal's font
		gtk_file = popen("cat $HOME/.minttyrc | grep '^Font=.*' | grep -o '[0-9A-z ]*$'", "r");
		fgets(font_str, MAX_STRLEN, gtk_file);
		pclose(gtk_file);
	}

	if (verbose)
	{
		VERBOSE_OUT("Found GTK as ", str);
	}

	return;
}

//fill_detected_arr
//fills an array of 15 strings with the data gathered from the detect functions
//WARNING: the order of the parameters is NOT the order of the array
void fill_detected_arr(char* arr[15], char* distro, char* arch, char* host, char* kernel, char* uptime, char* pkgs, char* cpu, char* gpu, char* disk, char* mem, char* shell, char* shell_vers, char* res, char* de, char* wm, char* wm_theme, char* gtk)
{
	arr[0] = host;
	arr[1] = distro;
	arr[2] = kernel;
	arr[3] = arch;
	arr[4] = cpu;
	arr[5] = gpu;
	arr[6] = shell; //figure out how to include shell_vers in here
	arr[7] = pkgs;
	arr[8] = disk;
	arr[9] = mem;
	arr[10] = uptime;
	arr[11] = res;
	arr[12] = de;
	arr[13] = wm;
	arr[14] = wm_theme;
	arr[15] = gtk;

	return;
}

/*  **  END DETECTION FUNCTIONS  **  */


/*  **  BEGIN AUXILIARY FUNCTIONS  **  */

//safe_strncpy
//calls strncpy with the given params, then inserts a null char at the last position
//returns a string containing the copied data
char* safe_strncpy(char* destination, const char* source, size_t num)
{
	char* ret = strncpy(destination, source, num);
	ret[num - 1] = '\0';
	return ret;
}

//split_uptime
//splits param uptime into individual time-units
//PREREQ: uptime _must_ be in seconds
void split_uptime(float uptime, int* secs, int* mins, int* hrs, int* days)
{
	*secs = (int) uptime % 60;
	*mins = (int) (uptime / 60) % 60;
	*hrs = (int) (uptime / 3600) % 24;
	*days = (int) (uptime / 86400);

	return;
}

/*  **  END AUXILIARY FUNCTIONS  **  */


/*  **  BEGIN FLAG/OUTPUT/MISC FUNCTIONS  **  */

//main_output
//the primary output for screenfetch-c - all info and ascii art is printed here
void main_output(void)
{
	if (OS == CYGWIN)
	{
		for (int i = 0; i < 16; i++)
		{
			printf("%s %s%s\n", windows_logo[i], detected_arr_names[i], detected_arr[i]);
		}
	}

	else if (OS == OSX)
	{
		for (int i = 0; i < 16; i++)
		{
			printf("%s %s%s\n", macosx_logo[i], detected_arr_names[i], detected_arr[i]);
		}
	}

	else if (OS == LINUX)
	{
		if (STRCMP(distro_str, "Arch Linux - Old"))
		{

		}

		else if (STRCMP(distro_str, "Arch Linux"))
		{

		}

		else if (STRCMP(distro_str, "LinuxMint"))
		{
			for (int i = 0; i < 16; i++)
			{
				printf("%s %s%s\n", mint_logo[i], detected_arr_names[i], detected_arr[i]);
			}
			//ugly fix
			printf("%s\n%s\n", mint_logo[16], mint_logo[17]);
		}

		else if (STRCMP(distro_str, "LMDE"))
		{
			for (int i = 0; i < 16; i++)
			{
				printf("%s %s%s\n", lmde_logo[i], detected_arr_names[i], detected_arr[i]);
			}
			//ugly fix
			printf("%s\n%s\n", lmde_logo[16], lmde_logo[17]);
		}

		else if (STRCMP(distro_str, "Ubuntu"))
		{
			for (int i = 0; i < 16; i++)
			{
				printf("%s %s%s\n", ubuntu_logo[i], detected_arr_names[i], detected_arr[i]);
			}
			//ugly fix
			printf("%s\n%s\n", ubuntu_logo[16], ubuntu_logo[17]);
		}

		else if (STRCMP(distro_str, "Debian"))
		{

		}

		else if (STRCMP(distro_str, "CrunchBang"))
		{

		}

		else if (STRCMP(distro_str, "Gentoo"))
		{

		}

		else if (STRCMP(distro_str, "Funtoo"))
		{

		}

		else if (STRCMP(distro_str, "Fedora"))
		{
			for (int i = 0; i < 16; i++)
			{
				printf("%s %s%s\n", fedora_logo[i], detected_arr_names[i], detected_arr[i]);
			}
			//ugly fix
			printf("%s\n%s\n", fedora_logo[16], fedora_logo[17]);
		}

		else if (STRCMP(distro_str, "Mandriva") || STRCMP(distro_str, "Mandrake"))
		{

		}

		else if (STRCMP(distro_str, "OpenSUSE"))
		{

		}

		else if (STRCMP(distro_str, "Slackware"))
		{

		}

		else if (STRCMP(distro_str, "Red Hat Linux"))
		{

		}

		else if (STRCMP(distro_str, "Frugalware"))
		{

		}

		else if (STRCMP(distro_str, "Peppermint"))
		{

		}

		else if (STRCMP(distro_str, "SolusOS"))
		{

		}

		else if (STRCMP(distro_str, "Mageia"))
		{

		}

		else if (STRCMP(distro_str, "ParabolaGNU/Linux-libre"))
		{

		}

		else if (STRCMP(distro_str, "Viperr"))
		{

		}

		else if (STRCMP(distro_str, "LinuxDeepin"))
		{

		}

		else if (STRCMP(distro_str, "Chakra"))
		{

		}

		else if (STRCMP(distro_str, "Fuduntu"))
		{

		}

		else if (STRCMP(distro_str, "Trisquel"))
		{

		}

		else if (STRCMP(distro_str, "Manjaro"))
		{

		}

		else if (STRCMP(distro_str, "Elementary OS"))
		{

		}

		else if (STRCMP(distro_str, "Scientific Linux"))
		{

		}

		else if (STRCMP(distro_str, "Backtrack Linux"))
		{

		}

		else if (STRCMP(distro_str, "Sabayon"))
		{

		}

		else if (STRCMP(distro_str, "Linux"))
		{

		}
	}

	else if (OS == FREEBSD)
	{
		for (int i = 0; i < 16; i++)
		{
			printf("%s %s%s\n", freebsd_logo[i], detected_arr_names[i], detected_arr[i]);
		}
		//ugly fix
		printf("%s\n%s\n", freebsd_logo[16], freebsd_logo[17]);
	}

	else if (OS == OPENBSD)
	{

	}

	else if (OS == NETBSD)
	{

	}

	else if (OS == DFBSD)
	{

	}

	return;
}

//display_version
//called if the -v flag is tripped, outputs the current version of screenfetch-c
void display_version(void)
{
	printf("%s\n", TBLU "screenfetch-c - Version 0.5 ALPHA");
	printf("%s\n", "Warning: This version of screenfetch is not yet finished");
	printf("%s\n", "and as such may contain bugs and security holes. Use with caution." TNRM);

	return;
}

//display_help
//called if the -h flag is tripped, tells the user where to find the manpage
void display_help(void)
{
	printf("%s\n", TBLU "screenfetch-c");
	printf("%s\n", "A rewrite of screenFetch, the popular shell script, in C.");
	printf("%s\n", "Operating Systems currently supported:");
	printf("%s\n", "Windows (via Cygwin), Linux, *BSD, and OS X.");
	printf("%s\n", "Using screenfetch-c on an OS not listed above may not work entirely or at all.");
	printf("%s\n", "Please access 'man screenfetch' for in-depth information on compatibility and usage." TNRM);
	return;
}

//take_screenshot
//takes a screenshot and saves it to $HOME 
//SECURITY WARNING: THIS FUNCTION MAKES SYSTEM CALLS
void take_screenshot(void)
{
	FILE* ss_file;

	if (OS == CYGWIN || OS == UNKNOWN)
	{
		//cygwin does not currently have a simple screenshot solution
		//potential solutions: "import -window root screenfetch_screenshot.jpg" - requires X
		ERROR_OUT("Error: ", "This program does not currently support screenshots on your OS.");
		return;
	}

	printf("%s", "Taking shot in 3..");
	fflush(stdout);
	sleep(1);
	printf("%s", "2..");
	fflush(stdout);
	sleep(1);
	printf("%s", "1..");
	fflush(stdout);
	sleep(1);
	printf("%s\n", "0");

	if (OS == OSX)
	{
		system("screencapture -x ~/screenfetch_screenshot.png 2> /dev/null");	
	}

	else if (OS == LINUX || ISBSD())
	{
		system("scrot ~/screenfetch_screenshot.png 2> /dev/null");
	}

	char* loc = getenv("HOME");
	strcat(loc, "/screenfetch_screenshot.png");
	ss_file = fopen(loc, "r");

	if (ss_file != NULL && verbose)
	{
		fclose(ss_file);
		VERBOSE_OUT("Screenshot successfully saved.", "");
	}
	
	else if (ss_file == NULL)
	{
		ERROR_OUT("Error: ", "Problem saving screenshot.");
	}

	return;
}

/*  **  END FLAG/OUTPUT/MISC FUNCTIONS  **  */
