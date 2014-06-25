#!/bin/sh

prefix=p
extensions=`grep gl $1 | sort`
supported=`grep GL $1 | sort`

cat <<END
#include <string.h>
#include "opengl.h"

END

for support in $supported; do
    echo "int $prefix$support;"
done

cat <<END

#if defined(CYGWIN) || defined(WINDOWS) || defined(LINUX)
END

for extension in $extensions; do
    upper="`echo $extension|tr '[a-z]' '[A-Z]'`"
    echo "PFN${upper}PROC $prefix$extension;"
done

cat <<END
#endif

void glext_init(void)
{
    static int initialized = 0;
    if (initialized)
        return;

    initialized = 1;

    char const * const extensions = (char *) glGetString(GL_EXTENSIONS);

END
# TODO extensions may be unused resulting in compilation error

for support in $supported; do
    echo "    $prefix$support = strstr(extensions, \"$support\") != 0;"
done

cat <<END

#if defined(CYGWIN) || defined(WINDOWS) || defined(LINUX)
END

for extension in $extensions; do
    upper="`echo $extension|tr '[a-z]' '[A-Z]'`"
#    echo "    ${extension} = (PFN${upper}PROC) glXGetProcAddress(\"$extension\");"
    echo "    ${extension} = (PFN${upper}PROC) wglGetProcAddress(\"$extension\");"
done

cat <<END
#endif
}
END
