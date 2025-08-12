// This is a test script to test the features of the flint engine under different conditions

// ps1 command: gcc testing/testing.c src/icd-functions/*.c src/state-functions/*.c src/struct-functions/*.c src/tooling/*.c -Iinclude -o test 
//              "path/to/location/of/db" | ./test.exe

// Include tooling
#include "../include/tooling/tooling.h"
#include <stdio.h>

int main() {
    char path[256];
    printf("Enter a path to create the db");
    scanf("%s", path);
    init(path);
}