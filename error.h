#ifndef ERROR_H
#define ERROR_H

#include <errno.h>

#include "opengl.h"

void warn(char const message[]);
void error_check_gl(void);
void error_check_gl_message(char const * message);
void error_check(int fail_condition, char const fail_message[]);
void error_check_arg(int fail_condition, char const fail_message[], char const argument[]);
char const * error_to_string(GLenum);
void error_print_stack(void);

char const * cl_error_to_string(int);

#define error_fail(message) error_check(1, message)

#endif
