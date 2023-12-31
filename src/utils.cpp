#include "utils.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/ioctl.h>
#include <termios.h>
#include <unistd.h>

void zip(const char *folder, const char *compress_path) {
    char command[1000];
    sprintf(command, "cd %s && zip -r -y ../%s . > /dev/null 2>&1", folder, compress_path);

    int result = system(command);

    if (result == 0) {
        printf("Folder zipped successfully.\n");
    } else {
        printf("Error zipping folder.\n");
    }
}

void unzip(const char *compressed_path, const char *extract_path) {
    char command[1000];
    sprintf(command, "unzip -o %s -d %s", compressed_path, extract_path);

    int result = system(command);

    if (result == 0) {
        printf("Folder unzipped successfully.\n");
    } else {
        printf("Error unzipping folder.\n");
    }
}

void print_centered(const char *text) {
    struct winsize w;
    ioctl(STDOUT_FILENO, TIOCGWINSZ, &w);

    int len = strlen(text);
    int spaces = (w.ws_col - len) / 2;

    for (int i = 0; i < spaces; i++) {
        printf(" ");
    }

    printf(ANSI_BOLD ANSI_COLOR_BLUE "%s" ANSI_RESET "\n", text);
}

void enable_raw_mode(struct termios *orig_termios) {
    struct termios raw = *orig_termios;
    raw.c_lflag &= ~(ECHO | ICANON);
    tcsetattr(STDIN_FILENO, TCSAFLUSH, &raw);
}

void disable_raw_mode(struct termios *orig_termios) {
    tcsetattr(STDIN_FILENO, TCSAFLUSH, orig_termios);
}

void print_menu(int current_selection, const char *options[], int num_options) {
    for (int i = 0; i < num_options; i++) {
        if (i == current_selection) {
            printf(ANSI_BOLD ANSI_COLOR_YELLOW "> %s" ANSI_RESET "\n", options[i]);

        } else {
            printf("  %s\n", options[i]);
        }
    }
}

int process_menu(const char *menu_items[], int num_items) {
    int current_selection = 0;

    struct termios orig_termios;
    tcgetattr(STDIN_FILENO, &orig_termios);

    enable_raw_mode(&orig_termios);
    print_centered("CPPDRIVE\n");
    print_menu(current_selection, menu_items, num_items);

    while (1) {
        char c;
        read(STDIN_FILENO, &c, 1);

        if (c == '\033') {
            char seq[3];
            read(STDIN_FILENO, &seq[0], 1);
            read(STDIN_FILENO, &seq[1], 1);

            if (seq[0] == '[') {
                if (seq[1] == 'A' && current_selection > 0) {   // Up arrow
                    current_selection--;
                } else if (seq[1] == 'B' && current_selection < num_items - 1) {   // Down arrow
                    current_selection++;
                }
            }

            printf("\033[H\033[J");
            print_centered("CPPDRIVE\n");
            print_menu(current_selection, menu_items, num_items);
        } else if (c == '\n') {
            break;
        }
    }

    disable_raw_mode(&orig_termios);
    return current_selection;
}

char *handle_prompt(char *cur_user, char *user_dir) {
    const std::string app_name = "cppdrive";
    std::vector<std::string> tokens;
    std::string token;
    std::string user_dir_str(user_dir);
    std::istringstream tokenStream(user_dir_str);

    while (getline(tokenStream, token, '/')) {
        if (!token.empty()) {
            tokens.push_back(token);
        }
    }

    // Find the index of cur_user in the tokens
    size_t user_index = std::string::npos;
    for (size_t i = 0; i < tokens.size(); ++i) {
        if (tokens[i] == cur_user) {
            user_index = i;
            break;
        }
    }

    char *prompt = (char *) malloc(sizeof(char) * 100);
    if (user_index == std::string::npos) {
        sprintf(prompt, "[%s@%s ~/]$ ", cur_user, app_name.c_str());
        return prompt;
    }

    // Construct the path from the user's directory onwards
    std::string path = "~/";
    for (size_t i = user_index + 1; i < tokens.size(); ++i) {
        path += tokens[i] + "/";
    }

    sprintf(prompt, "[%s@%s %s]$ ", cur_user, app_name.c_str(), path.c_str());
    return prompt;
}