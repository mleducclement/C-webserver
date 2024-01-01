cmake_minimum_required(VERSION 3.27)
project(webserver C)

set(CMAKE_C_STANDARD 11)

add_executable(webserver main.c src/my_hashtable.c src/my_linkedlist.c)
