#                                               -*- Autoconf -*-
# Process this file with autoconf to produce a configure script.

AC_PREREQ([2.69])
AC_INIT([libegdbm],[0.1],[baileyp2012@gmail.com])

AC_CONFIG_AUX_DIR([build])
AC_CONFIG_MACRO_DIR([m4])
AC_CONFIG_SRCDIR([libdbm/delete.c])
AC_CONFIG_HEADERS([config.h])

LT_PREREQ([2.2])
LT_INIT([dlopen])

AM_INIT_AUTOMAKE

# Checks for programs.
AC_PROG_CC
AC_PROG_CC_STDC
AC_PROG_INSTALL
AM_PROG_CC_C_O

# TODO: Update which headers count, here.

# Fail build if libs, v120 cli, and v120irqd cannot be built.
# These are $fail_headers and $fail_libs.
fail_headers='assert ctype errno fcntl getopt poll signal stdarg stdbool
 stddef stdint stdio stdlib string syslog sys/mman sys/ioctl sys/socket
 sys/types unistd'
for i in $fail_headers; do
  AC_CHECK_HEADER([${i}.h], ,[AC_MSG_ERROR([${i}.h missing])])
done

# # TODO: Robustulate this
# BASH_COMPLETION_DIR=`pkg-config --variable=completiondir bash-completion \
#      2>/dev/null`
# if test -z "${completiondir}"; then
#     BASH_COMPLETION_DIR="$datadir/bash-completion/completions"
# fi
# AC_SUBST([BASH_COMPLETION_DIR])
# echo "Bash completions sent to ${BASH_COMPLETION_DIR}"

# Checks for typedefs, structures, and compiler characteristics.
AC_HEADER_STDBOOL
AC_C_INLINE
AC_TYPE_UINT16_T
AC_TYPE_UINT32_T
AC_TYPE_UINT64_T
AC_TYPE_UINT8_T

# Checks for library functions.
AC_CHECK_FUNCS([strtoul strtoull])

AC_CONFIG_FILES([Makefile
                 libdbm/Makefile
                 include/Makefile
                 examples/Makefile])

# ==== Print final messages to user =====================================

AC_OUTPUT

echo \
"-------------------------------------------------

${PACKAGE_NAME} Version ${PACKAGE_VERSION}

Prefix: '${prefix}'.
Compiler: '${CC} ${CFLAGS} ${CPPFLAGS}'

Able to build libs?       yes

Now type 'make @<:@<target>@:>@'
    where the optional <target> is:
        all         - build all binaries
        install     - install everything

--------------------------------------------------"
