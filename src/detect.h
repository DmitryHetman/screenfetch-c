#ifndef SCREENFETCH_C_DETECT_H
#define SCREENFETCH_C_DETECT_H

void detect_distro(char *str, bool error);
void detect_arch(char *str);
void detect_host(char *str);
void detect_kernel(char *str);
void detect_uptime(char *str);
// void detect_pkgs(char *str);
void detect_cpu(char *str);
void detect_gpu(char *str, bool error);
void detect_disk(char *str);
void detect_mem(char *str);
void detect_shell(char *str, bool error);
void detect_res(char *str, bool error);
void detect_de(char *str);
void detect_wm(char *str);

#endif /* SCREENFETCH_C_DETECT_H */
