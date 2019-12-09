#pragma once

#define ON_SCOPE_EXIT(destructor) __attribute__((__cleanup__(destructor)))

#define _GNU_SOURCE

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>

#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/sem.h>
#include <sys/shm.h>

#define TOKEN_FILENAME "/tmp/lab6_tok_file"
#define TOKEN_ID 42