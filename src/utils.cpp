#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/ioctl.h>
#include <termios.h>
#include <unistd.h>

#include "util.h"

#define UP_ARROW   "\033[A"
#define DOWN_ARROW "\033[B"

#define ANSI_COLOR_BLUE   "\033[34m"
#define ANSI_COLOR_YELLOW "\033[33m"
#define ANSI_BOLD         "\033[1m"
#define ANSI_RESET        "\033[0m"

void zipFolder(const char *folderPath, const char *zipPath) {
    char command[1000];
    sprintf(command, "cd %s && zip -r -y ../%s .", folderPath, zipPath);

    int result = system(command);

    if (result == 0) {
        printf("Folder zipped successfully.\n");
    } else {
        printf("Error zipping folder.\n");
    }
}

// Function to unzip a folder
void unzipFolder(const char *zipPath, const char *extractPath) {
    char command[1000];
    sprintf(command, "unzip -o %s -d %s", zipPath, extractPath);

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
    printf("You selected: %s\n", menu_items[current_selection]);
    return current_selection;
}