#ifndef __VALIDATE_H__
#define __VALIDATE_H__

#define INVALID_IP -1

/**
 * Validate ip address
 * @param ip ip address
 * @return 0 if valid, -1 if invalid
 */
int validate_ip(const char *ip);

/**
 * validate path is file or directory
 * @param path path to validate
 * @return 0 if valid, -1 if invalid
 */
int validate_file_or_dir(const char *path);

#endif   // !
