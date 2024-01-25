cmake_minimum_required(VERSION 3.22)
project(webserver C)

set(CMAKE_C_STANDARD 11)

add_executable(webserver main.c
        src/my_linkedlist.c
        main.c
        src/my_hashtable.c
        src/constants.h
        src/toolbox.c
        src/toolbox.h)
