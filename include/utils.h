#ifndef __UTILS_H__
#define __UTILS_H__

#include "color.h"

#define UP_ARROW   "\033[A"
#define DOWN_ARROW "\033[B"

void zipFolder(const char *folderPath, const char *zipPath);
void unzipFolder(const char *zipPath, const char *extractPath);

/**
 * print a string centered on the screen
 @param text the string to print
*/
void print_centered(const char *text);

/**
 * Print menu and get user selection
 * @param menu_items array of options to display
 * @param num_items number of options
 */
int process_menu(const char *menu_items[], int num_items);

#endif   // !__UTILS