#ifndef TERMINAL_H
#define TERMINAL_H

#include <vector>

#include "file_tree.h"

enum class COMMAND { LS, CD, UPLOAD, DOWNLOAD, EXIT, INVALID, CLEAR, MKDIR };

struct Command {
    COMMAND command;
    std::vector<std::string> argument;
};

struct Command parseCommand(const std::string &command);

class Terminal {
   public:
    Terminal(const std::string &treeData);

    Terminal(FileTree root);

    void resetCurrentDirectory();

    void ls() const;

    int cd(const std::string &directory);

   private:
    FileTree root;
    FileTree currentDirectory;
};

#endif   // TERMINAL_H
