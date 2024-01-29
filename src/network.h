//
// Created by pres-mleducclement on 2024-01-29.
//

#ifndef NETWORK_H
#define NETWORK_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <arpa/inet.h>
#include "constants.h"
#include "toolbox.h"
#include "my_hashtable.h"

void startWebserver();

#endif //NETWORK_H
