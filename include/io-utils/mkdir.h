#ifndef MKDIR_H
#define MKDIR_H

#ifdef _WIN32
#include <windows.h>
#else
#include <sys/stat.h>
#include <errno.h>
#endif

int mk_dir(const char* path) {
    #ifdef _WIN32
        if (CreateDirectory((LPCWSTR) path, NULL) || GetLastError() == ERROR_ALREADY_EXISTS) {
            return 0;
        } else {
            return 1;
        }
    #else
        if (mkdir(path, 0755) == 0 || errno == EEXIST) {
            return 0;
        } else {
            perror("mkdir");
            return 1;
        }
    #endif
}

#endif