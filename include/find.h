#ifndef __FIND_H__
#define __FIND_H__

struct SearchResult_ {
    int count;
    char **files;
};

typedef struct SearchResult_ SearchResult;

SearchResult searchInDirectory(char *dirPath, char *fileName);
void ftserve_find(int sock_control, int sock_data, char *filename);

#endif   // !
