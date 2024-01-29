//
// Created by pres-mleducclement on 2024-01-25.
//

#ifndef TOOLBOX_H
#define TOOLBOX_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include "constants.h"
#include "my_hashtable.h"

hashtable* parseJSON(const char *jsonString);
int getValueType(const char *value);
void toLowerCase(char *str);

#endif //TOOLBOX_H
