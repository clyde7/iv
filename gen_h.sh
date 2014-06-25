#!/bin/sh

prefix=p
extensions=`grep gl $1 | sort`
supported=`grep GL $1 | sort`

cat <<END
#ifndef GEN_EXT_H
#define GEN_EXT_H

#include "glext.h"

END

for support in $supported; do
    cat <<END
#ifdef $support
#undef $support
#define $support $prefix$support
#else
#define $support (0)
#endif

END
done

echo "#if defined(CYGWIN) || defined(WINDOWS) || defined(LINUX)"
for extension in $extensions; do
    echo "#define ${extension} $prefix$extension"
done
echo "#endif"

for support in $supported; do
    echo "extern int $prefix$support;"
done

cat <<END

void glext_init(void);

#if defined(CYGWIN) || defined(WINDOWS) || defined(LINUX)
END

for extension in $extensions; do
    upper="`echo $extension|tr '[a-z]' '[A-Z]'`"
    echo "extern PFN${upper}PROC $prefix$extension;"
done

echo "#endif"
echo "#endif"

