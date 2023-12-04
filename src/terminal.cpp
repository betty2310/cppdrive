#include "terminal.h"

#include <iostream>

struct Command parseCommand(const std::string &command) {
    Command parsedCommand;
    std::string arg;
    bool isArg = false;
    for (const auto &c : command) {
        if (c == ' ') {
            if (isArg) {
                parsedCommand.argument.push_back(arg);
                arg = "";
                isArg = false;
            }
        } else {
            if (!isArg) {
                isArg = true;
            }
            arg += c;
        }
    }
    if (isArg) {
        parsedCommand.argument.push_back(arg);
    }

    if (parsedCommand.argument.size() == 1) {
        if (parsedCommand.argument[0] == "ls") {
            parsedCommand.command = COMMAND::LS;
        } else if (parsedCommand.argument[0] == "exit") {
            parsedCommand.command = COMMAND::EXIT;
        } else if (parsedCommand.argument[0] == "clear") {
            parsedCommand.command = COMMAND::CLEAR;
        } else {
            parsedCommand.command = COMMAND::INVALID;
        }
    } else {
        if (parsedCommand.argument[0] == "cd") {
            parsedCommand.command = COMMAND::CD;
        } else if (parsedCommand.argument[0] == "upload") {
            parsedCommand.command = COMMAND::UPLOAD;
        } else if (parsedCommand.argument[0] == "download") {
            parsedCommand.command = COMMAND::DOWNLOAD;
        } else {
            parsedCommand.command = COMMAND::INVALID;
        }
    }

    return parsedCommand;
}

Terminal::Terminal(const std::string &treeData) {
    root = root.parseTree(treeData);
    currentDirectory = root;
}

Terminal::Terminal(FileTree root) : root(root), currentDirectory(root) {}

void Terminal::ls() const { currentDirectory.display(); }

int Terminal::cd(const std::string &directory) {
    if (directory == "..") {
        if (currentDirectory.name == ".") {
            std::cout << "Already at root directory" << std::endl;
            return 0;
        }
        currentDirectory = root;
        return 1;
    }

    bool found = false;

    // Find the child directory with the given name
    for (const auto &child : currentDirectory.children) {
        if (child.name == directory && !child.children.empty()) {
            currentDirectory = child;   // Change to the child directory
            found = true;
            break;
        }
    }

    if (!found) {
        std::cout << "Directory not found: " << directory << std::endl;
    }

    return found;
}

void Terminal::resetCurrentDirectory() { currentDirectory = root; }