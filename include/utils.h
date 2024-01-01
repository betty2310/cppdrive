#ifndef __UTILS_H__
#define __UTILS_H__

#include <sstream>
#include <string>
#include <vector>

#include "color.h"

#define UP_ARROW   "\033[A"
#define DOWN_ARROW "\033[B"

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

/**
 * get prompt string from current user and current directory
 * @param cur_user current user
 * @param user_dir current directory
 * @return prompt string
 */
char *handle_prompt(char *cur_user, char *user_dir);

/**
 * Split a string by a delimiter
 * @param s string to split
 * @param delim delimiter
 * @return vector of strings
 */
std::vector<std::string> split(const std::string &s, char delim);

/**
 * compress a folder
 * @param folder path to folder
 * @param compress_path path to compressed file
 */
void zip(const char *folder, const char *compress_path);

/**
 * decompress a folder
 * @param compressed_path path to compressed folder
 * @param extract_path path to extract folder
 */
void unzip(const char *compressed_path, const char *extract_path);

#endif   // !__UTILS