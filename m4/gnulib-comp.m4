# DO NOT EDIT! GENERATED AUTOMATICALLY!
# Copyright (C) 2002-2014 Free Software Foundation, Inc.
#
# This file is free software; you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation; either version 3 of the License, or
# (at your option) any later version.
#
# This file is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with this file.  If not, see <http://www.gnu.org/licenses/>.
#
# As a special exception to the GNU General Public License,
# this file may be distributed as part of a program that
# contains a configuration script generated by Autoconf, under
# the same distribution terms as the rest of that program.
#
# Generated by gnulib-tool.
#
# This file represents the compiled summary of the specification in
# gnulib-cache.m4. It lists the computed macro invocations that need
# to be invoked from configure.ac.
# In projects that use version control, this file can be treated like
# other built files.


# This macro should be invoked from ./configure.ac, in the section
# "Checks for programs", right after AC_PROG_CC, and certainly before
# any checks for libraries, header files, types and library functions.
AC_DEFUN([gl_EARLY],
[
  m4_pattern_forbid([^gl_[A-Z]])dnl the gnulib macro namespace
  m4_pattern_allow([^gl_ES$])dnl a valid locale name
  m4_pattern_allow([^gl_LIBOBJS$])dnl a variable
  m4_pattern_allow([^gl_LTLIBOBJS$])dnl a variable
  AC_REQUIRE([gl_PROG_AR_RANLIB])
  # Code from module absolute-header:
  # Code from module errno:
  # Code from module extensions:
  AC_REQUIRE([gl_USE_SYSTEM_EXTENSIONS])
  # Code from module extern-inline:
  # Code from module fcntl-h:
  # Code from module fd-hook:
  # Code from module forkpty:
  # Code from module include_next:
  # Code from module ioctl:
  # Code from module lib-ignore:
  # Code from module login_tty:
  # Code from module msvc-inval:
  # Code from module msvc-nothrow:
  # Code from module openpty:
  # Code from module posix_openpt:
  # Code from module pty:
  # Code from module snippet/_Noreturn:
  # Code from module snippet/arg-nonnull:
  # Code from module snippet/c++defs:
  # Code from module snippet/warn-on-use:
  # Code from module socklen:
  # Code from module ssize_t:
  # Code from module stdalign:
  # Code from module stddef:
  # Code from module stdlib:
  # Code from module string:
  # Code from module strndup:
  # Code from module strnlen:
  # Code from module sys_ioctl:
  # Code from module sys_socket:
  # Code from module sys_types:
  # Code from module sys_uio:
  # Code from module unistd:
])

# This macro should be invoked from ./configure.ac, in the section
# "Check for header files, types and library functions".
AC_DEFUN([gl_INIT],
[
  AM_CONDITIONAL([GL_COND_LIBTOOL], [true])
  gl_cond_libtool=true
  gl_m4_base='m4'
  m4_pushdef([AC_LIBOBJ], m4_defn([gl_LIBOBJ]))
  m4_pushdef([AC_REPLACE_FUNCS], m4_defn([gl_REPLACE_FUNCS]))
  m4_pushdef([AC_LIBSOURCES], m4_defn([gl_LIBSOURCES]))
  m4_pushdef([gl_LIBSOURCES_LIST], [])
  m4_pushdef([gl_LIBSOURCES_DIR], [])
  gl_COMMON
  gl_source_base='lib'
  gl_HEADER_ERRNO_H
  AC_REQUIRE([gl_EXTERN_INLINE])
  gl_FCNTL_H
  gl_FUNC_FORKPTY
  if test $HAVE_FORKPTY = 0 || test $REPLACE_FORKPTY = 1; then
    AC_LIBOBJ([forkpty])
  fi
  gl_PTY_MODULE_INDICATOR([forkpty])
  gl_FUNC_IOCTL
  if test $HAVE_IOCTL = 0 || test $REPLACE_IOCTL = 1; then
    AC_LIBOBJ([ioctl])
  fi
  gl_SYS_IOCTL_MODULE_INDICATOR([ioctl])
  gl_IGNORE_UNUSED_LIBRARIES
  gl_FUNC_LOGIN_TTY
  if test $ac_cv_func_login_tty = no; then
    AC_LIBOBJ([login_tty])
  fi
  gl_PTY_MODULE_INDICATOR([login_tty])
  gl_MSVC_INVAL
  if test $HAVE_MSVC_INVALID_PARAMETER_HANDLER = 1; then
    AC_LIBOBJ([msvc-inval])
  fi
  gl_MSVC_NOTHROW
  if test $HAVE_MSVC_INVALID_PARAMETER_HANDLER = 1; then
    AC_LIBOBJ([msvc-nothrow])
  fi
  gl_FUNC_OPENPTY
  if test $HAVE_OPENPTY = 0 || test $REPLACE_OPENPTY = 1; then
    AC_LIBOBJ([openpty])
  fi
  gl_PTY_MODULE_INDICATOR([openpty])
  gl_FUNC_POSIX_OPENPT
  if test $HAVE_POSIX_OPENPT = 0; then
    AC_LIBOBJ([posix_openpt])
  fi
  gl_STDLIB_MODULE_INDICATOR([posix_openpt])
  gl_PTY_H
  gl_TYPE_SOCKLEN_T
  gt_TYPE_SSIZE_T
  gl_STDALIGN_H
  gl_STDDEF_H
  gl_STDLIB_H
  gl_HEADER_STRING_H
  gl_FUNC_STRNDUP
  if test $HAVE_STRNDUP = 0 || test $REPLACE_STRNDUP = 1; then
    AC_LIBOBJ([strndup])
  fi
  gl_STRING_MODULE_INDICATOR([strndup])
  gl_FUNC_STRNLEN
  if test $HAVE_DECL_STRNLEN = 0 || test $REPLACE_STRNLEN = 1; then
    AC_LIBOBJ([strnlen])
    gl_PREREQ_STRNLEN
  fi
  gl_STRING_MODULE_INDICATOR([strnlen])
  gl_SYS_IOCTL_H
  AC_PROG_MKDIR_P
  gl_HEADER_SYS_SOCKET
  AC_PROG_MKDIR_P
  gl_SYS_TYPES_H
  AC_PROG_MKDIR_P
  gl_HEADER_SYS_UIO
  AC_PROG_MKDIR_P
  gl_UNISTD_H
  # End of code from modules
  m4_ifval(gl_LIBSOURCES_LIST, [
    m4_syscmd([test ! -d ]m4_defn([gl_LIBSOURCES_DIR])[ ||
      for gl_file in ]gl_LIBSOURCES_LIST[ ; do
        if test ! -r ]m4_defn([gl_LIBSOURCES_DIR])[/$gl_file ; then
          echo "missing file ]m4_defn([gl_LIBSOURCES_DIR])[/$gl_file" >&2
          exit 1
        fi
      done])dnl
      m4_if(m4_sysval, [0], [],
        [AC_FATAL([expected source file, required through AC_LIBSOURCES, not found])])
  ])
  m4_popdef([gl_LIBSOURCES_DIR])
  m4_popdef([gl_LIBSOURCES_LIST])
  m4_popdef([AC_LIBSOURCES])
  m4_popdef([AC_REPLACE_FUNCS])
  m4_popdef([AC_LIBOBJ])
  AC_CONFIG_COMMANDS_PRE([
    gl_libobjs=
    gl_ltlibobjs=
    if test -n "$gl_LIBOBJS"; then
      # Remove the extension.
      sed_drop_objext='s/\.o$//;s/\.obj$//'
      for i in `for i in $gl_LIBOBJS; do echo "$i"; done | sed -e "$sed_drop_objext" | sort | uniq`; do
        gl_libobjs="$gl_libobjs $i.$ac_objext"
        gl_ltlibobjs="$gl_ltlibobjs $i.lo"
      done
    fi
    AC_SUBST([gl_LIBOBJS], [$gl_libobjs])
    AC_SUBST([gl_LTLIBOBJS], [$gl_ltlibobjs])
  ])
  gltests_libdeps=
  gltests_ltlibdeps=
  m4_pushdef([AC_LIBOBJ], m4_defn([gltests_LIBOBJ]))
  m4_pushdef([AC_REPLACE_FUNCS], m4_defn([gltests_REPLACE_FUNCS]))
  m4_pushdef([AC_LIBSOURCES], m4_defn([gltests_LIBSOURCES]))
  m4_pushdef([gltests_LIBSOURCES_LIST], [])
  m4_pushdef([gltests_LIBSOURCES_DIR], [])
  gl_COMMON
  gl_source_base='tests'
changequote(,)dnl
  gltests_WITNESS=IN_`echo "${PACKAGE-$PACKAGE_TARNAME}" | LC_ALL=C tr abcdefghijklmnopqrstuvwxyz ABCDEFGHIJKLMNOPQRSTUVWXYZ | LC_ALL=C sed -e 's/[^A-Z0-9_]/_/g'`_GNULIB_TESTS
changequote([, ])dnl
  AC_SUBST([gltests_WITNESS])
  gl_module_indicator_condition=$gltests_WITNESS
  m4_pushdef([gl_MODULE_INDICATOR_CONDITION], [$gl_module_indicator_condition])
  m4_popdef([gl_MODULE_INDICATOR_CONDITION])
  m4_ifval(gltests_LIBSOURCES_LIST, [
    m4_syscmd([test ! -d ]m4_defn([gltests_LIBSOURCES_DIR])[ ||
      for gl_file in ]gltests_LIBSOURCES_LIST[ ; do
        if test ! -r ]m4_defn([gltests_LIBSOURCES_DIR])[/$gl_file ; then
          echo "missing file ]m4_defn([gltests_LIBSOURCES_DIR])[/$gl_file" >&2
          exit 1
        fi
      done])dnl
      m4_if(m4_sysval, [0], [],
        [AC_FATAL([expected source file, required through AC_LIBSOURCES, not found])])
  ])
  m4_popdef([gltests_LIBSOURCES_DIR])
  m4_popdef([gltests_LIBSOURCES_LIST])
  m4_popdef([AC_LIBSOURCES])
  m4_popdef([AC_REPLACE_FUNCS])
  m4_popdef([AC_LIBOBJ])
  AC_CONFIG_COMMANDS_PRE([
    gltests_libobjs=
    gltests_ltlibobjs=
    if test -n "$gltests_LIBOBJS"; then
      # Remove the extension.
      sed_drop_objext='s/\.o$//;s/\.obj$//'
      for i in `for i in $gltests_LIBOBJS; do echo "$i"; done | sed -e "$sed_drop_objext" | sort | uniq`; do
        gltests_libobjs="$gltests_libobjs $i.$ac_objext"
        gltests_ltlibobjs="$gltests_ltlibobjs $i.lo"
      done
    fi
    AC_SUBST([gltests_LIBOBJS], [$gltests_libobjs])
    AC_SUBST([gltests_LTLIBOBJS], [$gltests_ltlibobjs])
  ])
])

# Like AC_LIBOBJ, except that the module name goes
# into gl_LIBOBJS instead of into LIBOBJS.
AC_DEFUN([gl_LIBOBJ], [
  AS_LITERAL_IF([$1], [gl_LIBSOURCES([$1.c])])dnl
  gl_LIBOBJS="$gl_LIBOBJS $1.$ac_objext"
])

# Like AC_REPLACE_FUNCS, except that the module name goes
# into gl_LIBOBJS instead of into LIBOBJS.
AC_DEFUN([gl_REPLACE_FUNCS], [
  m4_foreach_w([gl_NAME], [$1], [AC_LIBSOURCES(gl_NAME[.c])])dnl
  AC_CHECK_FUNCS([$1], , [gl_LIBOBJ($ac_func)])
])

# Like AC_LIBSOURCES, except the directory where the source file is
# expected is derived from the gnulib-tool parameterization,
# and alloca is special cased (for the alloca-opt module).
# We could also entirely rely on EXTRA_lib..._SOURCES.
AC_DEFUN([gl_LIBSOURCES], [
  m4_foreach([_gl_NAME], [$1], [
    m4_if(_gl_NAME, [alloca.c], [], [
      m4_define([gl_LIBSOURCES_DIR], [lib])
      m4_append([gl_LIBSOURCES_LIST], _gl_NAME, [ ])
    ])
  ])
])

# Like AC_LIBOBJ, except that the module name goes
# into gltests_LIBOBJS instead of into LIBOBJS.
AC_DEFUN([gltests_LIBOBJ], [
  AS_LITERAL_IF([$1], [gltests_LIBSOURCES([$1.c])])dnl
  gltests_LIBOBJS="$gltests_LIBOBJS $1.$ac_objext"
])

# Like AC_REPLACE_FUNCS, except that the module name goes
# into gltests_LIBOBJS instead of into LIBOBJS.
AC_DEFUN([gltests_REPLACE_FUNCS], [
  m4_foreach_w([gl_NAME], [$1], [AC_LIBSOURCES(gl_NAME[.c])])dnl
  AC_CHECK_FUNCS([$1], , [gltests_LIBOBJ($ac_func)])
])

# Like AC_LIBSOURCES, except the directory where the source file is
# expected is derived from the gnulib-tool parameterization,
# and alloca is special cased (for the alloca-opt module).
# We could also entirely rely on EXTRA_lib..._SOURCES.
AC_DEFUN([gltests_LIBSOURCES], [
  m4_foreach([_gl_NAME], [$1], [
    m4_if(_gl_NAME, [alloca.c], [], [
      m4_define([gltests_LIBSOURCES_DIR], [tests])
      m4_append([gltests_LIBSOURCES_LIST], _gl_NAME, [ ])
    ])
  ])
])

# This macro records the list of files which have been installed by
# gnulib-tool and may be removed by future gnulib-tool invocations.
AC_DEFUN([gl_FILE_LIST], [
  build-aux/snippet/_Noreturn.h
  build-aux/snippet/arg-nonnull.h
  build-aux/snippet/c++defs.h
  build-aux/snippet/warn-on-use.h
  lib/errno.in.h
  lib/fcntl.in.h
  lib/fd-hook.c
  lib/fd-hook.h
  lib/forkpty.c
  lib/ioctl.c
  lib/login_tty.c
  lib/msvc-inval.c
  lib/msvc-inval.h
  lib/msvc-nothrow.c
  lib/msvc-nothrow.h
  lib/openpty.c
  lib/posix_openpt.c
  lib/pty.in.h
  lib/stdalign.in.h
  lib/stddef.in.h
  lib/stdlib.in.h
  lib/string.in.h
  lib/strndup.c
  lib/strnlen.c
  lib/sys_ioctl.in.h
  lib/sys_socket.c
  lib/sys_socket.in.h
  lib/sys_types.in.h
  lib/sys_uio.in.h
  lib/unistd.c
  lib/unistd.in.h
  lib/w32sock.h
  m4/00gnulib.m4
  m4/absolute-header.m4
  m4/errno_h.m4
  m4/extensions.m4
  m4/extern-inline.m4
  m4/fcntl-o.m4
  m4/fcntl_h.m4
  m4/gnulib-common.m4
  m4/include_next.m4
  m4/ioctl.m4
  m4/lib-ignore.m4
  m4/msvc-inval.m4
  m4/msvc-nothrow.m4
  m4/off_t.m4
  m4/onceonly.m4
  m4/posix_openpt.m4
  m4/pty.m4
  m4/pty_h.m4
  m4/socklen.m4
  m4/sockpfaf.m4
  m4/ssize_t.m4
  m4/stdalign.m4
  m4/stddef_h.m4
  m4/stdlib_h.m4
  m4/string_h.m4
  m4/strndup.m4
  m4/strnlen.m4
  m4/sys_ioctl_h.m4
  m4/sys_socket_h.m4
  m4/sys_types_h.m4
  m4/sys_uio_h.m4
  m4/unistd_h.m4
  m4/warn-on-use.m4
  m4/wchar_t.m4
])
