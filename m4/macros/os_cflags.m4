dnl ######################################################################
dnl Specify additional compile options based on the OS and the compiler
AC_DEFUN(AC_OS_CFLAGS,
[
AC_CACHE_CHECK(additional compiler flags,
ac_cv_os_cflags,
[
case "${host_os}" in
	irix6* )
		case "${CC}" in
			cc )
				# do not use 64-bit compiler
				ac_cv_os_cflags="-32 -Wl,-woff,84"
				;;
		esac
		;;
	osf4* | osf5* )
		# get the right version of struct sockaddr
		case "${CC}" in
			cc )
				ac_cv_os_cflags="-std -D_SOCKADDR_LEN"
				;;
			* )
				ac_cv_os_cflags="-D_SOCKADDR_LEN"
				;;
		esac
		;;
	osf* )
		# get the right version of struct sockaddr
		case "${CC}" in
			cc )
				ac_cv_os_cflags="-std -D_SOCKADDR_LEN -D_NO_PROTO"
				;;
			* )
				ac_cv_os_cflags="-D_SOCKADDR_LEN -D_NO_PROTO"
				;;
		esac
		;;
changequote(<<, >>)dnl
	aix4.[3-9]* )
changequote([, ])dnl
		# avoid circular dependencies in yp headers
#		ac_cv_os_cflags="-D_NO_PROTO -DHAVE_BAD_HEADERS"
#		ac_cv_os_cflags="-DHAVE_BAD_HEADERS"
		ac_cv_os_cflags="-DHAVE_BAD_HEADERS -D_XOPEN_EXTENDED_SOURCE"
		;;
	aix4.* )
		# turn on additional headers
		ac_cv_os_cflags="-D_XOPEN_EXTENDED_SOURCE"
		;;
changequote(<<, >>)dnl
	solaris2.[6-9]* | sunos5.[6-9]* )
changequote([, ])dnl
		# turn on 64-bit file offset interface
		case "${CC}" in
			* )
				ac_cv_os_cflags="-D_LARGEFILE64_SOURCE"
				;;
		esac
		;;
	hpux* )
		# use Ansi compiler on HPUX
		case "${CC}" in
			cc )
				ac_cv_os_cflags="-Ae"
				;;
		esac
		;;
	OFF-sunos4* )
		# make sure passing whole structures is handled in gcc
		case "${CC}" in
			gcc )
				ac_cv_os_cflags="-fpcc-struct-return"
				;;
		esac
		;;
	* )	ac_cv_os_cflags="" ;;
esac
])
CFLAGS="$CFLAGS $ac_cv_os_cflags"
])
dnl ======================================================================
