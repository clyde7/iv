#ifndef DEBUG_H
#define DEBUG_H

#include "string.h"

#define FILE_NAME(filename) (verbose ? filename : basename_(filename))

extern int debug;
extern int verbose;

#endif
