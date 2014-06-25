#include <execinfo.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#ifdef OPENCL
#include <OpenCL/opencl.h>
#endif

#include "error.h"
#include "print.h"
#include "opengl.h"

void warn(char const message[])
{
    printf(ANSI_RED "%s" ANSI_RESET "\n", message);
}

void error_check_gl(void)
{
    GLenum error = glGetError();
    error_check_arg(error, "OpenGL error: %s", error_to_string(error));
}

void error_check_gl_message(char const message[])
{
    fprintf(stderr, "BEFORE %s\n", message);
    error_check_gl();
    fprintf(stderr, "AFTER %s\n", message);
}

void error_check(int fail_condition, char const fail_message[])
{
    if (! fail_condition)
        return;

    fprintf(stderr, "%s\n", fail_message);
    exit(EXIT_FAILURE);
}

void error_check_arg(int fail_condition, char const fail_message[], char const argument[])
{
    if (! fail_condition)
        return;

    char buffer[512];
    sprintf(buffer, "%s\n", fail_message);
    fprintf(stderr, buffer, argument);
    exit(EXIT_FAILURE);
}

char const * error_to_string(GLenum error)
{
    switch (error)
    {
        case GL_NO_ERROR:          return "no error";
        case GL_INVALID_ENUM:      return "invalid enum";
        case GL_INVALID_VALUE:     return "invalid value";
        case GL_INVALID_OPERATION: return "invalid operation";
        case GL_STACK_OVERFLOW:    return "stack overflow";
        case GL_STACK_UNDERFLOW:   return "stack underflow";
        case GL_OUT_OF_MEMORY:     return "out of memory";
        case GL_TABLE_TOO_LARGE:   return "table too large";
    }

    return "unknown OpenGL error";
}

void error_print_stack(void)
{
    void * buffer[256];

//    backtrace_symbols_fd(buffer, 256, STDOUT_FILENO);

    int pointer_count = backtrace(buffer, 256);
    char ** strings = backtrace_symbols(buffer, pointer_count);
    if (strings)
    {
        for (int i = 0; i != pointer_count; ++ i)
        {
            printf("%s\n", strings[i]);
        }
    }
}

char const * c_error_to_string(int error)
{
    switch (error)
    {
        default:
        case 0:
            return NULL;

        case EDOM:   return "dom";
        case ERANGE: return "range";
        case EPERM:  return "operation not permitted";
        case ENOENT: return "no such file or directory";
    }
}

#ifdef OPENCL
char const * cl_error_to_string(int error)
{
    switch (error)
    {
        case CL_SUCCESS:                        return "no error";
        case CL_DEVICE_NOT_FOUND:               return "device not found";
        case CL_DEVICE_NOT_AVAILABLE:           return "device not available";
        case CL_COMPILER_NOT_AVAILABLE:         return "compiler not available";
        case CL_MEM_OBJECT_ALLOCATION_FAILURE:  return "memory object allocation failed";
        case CL_OUT_OF_RESOURCES:               return "out of resources";
        case CL_OUT_OF_HOST_MEMORY:             return "out of host memory";
        case CL_PROFILING_INFO_NOT_AVAILABLE:   return "unavailable profiling info";
        case CL_MEM_COPY_OVERLAP:               return "memory copy overlaps";
        case CL_IMAGE_FORMAT_MISMATCH:          return "image format mismatch";
        case CL_IMAGE_FORMAT_NOT_SUPPORTED:     return "image format unsupported";
        case CL_BUILD_PROGRAM_FAILURE:          return "building program failed";
        case CL_MAP_FAILURE:                    return "map failure";

        case CL_INVALID_VALUE:                  return "invalid value";
        case CL_INVALID_DEVICE_TYPE:            return "invalid device type";
        case CL_INVALID_PLATFORM:               return "invalid platform";
        case CL_INVALID_DEVICE:                 return "invalid device";
        case CL_INVALID_CONTEXT:                return "invalid context";
        case CL_INVALID_QUEUE_PROPERTIES:       return "invalid queue properties";
        case CL_INVALID_COMMAND_QUEUE:          return "invalid command queue";
        case CL_INVALID_HOST_PTR:               return "invalid host pointer";
        case CL_INVALID_MEM_OBJECT:             return "invalid memory object";
        case CL_INVALID_IMAGE_FORMAT_DESCRIPTOR: return "invalid image format descriptor";
        case CL_INVALID_IMAGE_SIZE:             return "invalid image size";
        case CL_INVALID_SAMPLER:                return "invalid sampler";
        case CL_INVALID_BINARY:                 return "invalid binary";
        case CL_INVALID_BUILD_OPTIONS:          return "invalid build options";
        case CL_INVALID_PROGRAM:                return "invalid program";
        case CL_INVALID_PROGRAM_EXECUTABLE:     return "invalid program executable";
        case CL_INVALID_KERNEL_NAME:            return "invalid kernel name";
        case CL_INVALID_KERNEL_DEFINITION:      return "invalid kernel definition";
        case CL_INVALID_KERNEL:                 return "invalid kernel";
        case CL_INVALID_ARG_INDEX:              return "invalid argument index";
        case CL_INVALID_ARG_VALUE:              return "invalid argument value";
        case CL_INVALID_ARG_SIZE:               return "invalid argument size";
        case CL_INVALID_KERNEL_ARGS:            return "invalid kernel arguments";
        case CL_INVALID_WORK_DIMENSION:         return "invalid work dimension";
        case CL_INVALID_WORK_GROUP_SIZE:        return "invalid work group size";
        case CL_INVALID_WORK_ITEM_SIZE:         return "invalid work item size";
        case CL_INVALID_GLOBAL_OFFSET:          return "invalid global offset";
        case CL_INVALID_EVENT_WAIT_LIST:        return "invalid event wait list";
        case CL_INVALID_EVENT:                  return "invalid event";
        case CL_INVALID_OPERATION:              return "invalid operation";
        case CL_INVALID_GL_OBJECT:              return "invalid OpenGL object";
        case CL_INVALID_BUFFER_SIZE:            return "invalid buffer size";
        case CL_INVALID_MIP_LEVEL:              return "invalid mip map level";
//        case CL_INVALID_GLOBAL_WORK_SIZE:       return "invalid global work size"; // new?
    }

    return "unknown OpenCL error";
}
#endif
