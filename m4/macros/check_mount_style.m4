dnl ######################################################################
dnl check style of mounting filesystems
AC_DEFUN(AC_CHECK_MOUNT_STYLE,
[
AC_CACHE_CHECK(style of mounting filesystems,
ac_cv_style_mount,
[
# select the correct style for mounting filesystems
case "${host_os_name}" in
	solaris1* | sunos[[34]]* | bsdi[[12]]* )
			ac_cv_style_mount=default ;;
	hpux9* | hpux10* )
			ac_cv_style_mount=hpux ;;
	svr4* | sysv4* | solaris* | sunos* | aoi* | hpux* )
			ac_cv_style_mount=svr4 ;;
	bsdi* )
			ac_cv_style_mount=bsdi3 ;;
	aix* )
			ac_cv_style_mount=aix ;;
	hpux* )
			ac_cv_style_mount=hpux ;;
	irix* )
			ac_cv_style_mount=irix6 ;;
	irix5* )
			ac_cv_style_mount=irix5 ;;
	isc3* )
			ac_cv_style_mount=isc3 ;;
	linux* )
			ac_cv_style_mount=linux ;;
	mach3* )
			ac_cv_style_mount=mach3 ;;
	stellix* )
			ac_cv_style_mount=stellix ;;
	* )	# no style needed.  Use default filesystem calls ala BSD
			ac_cv_style_mount=default ;;
esac
])
am_utils_mount_style_file="mountutil.c"
am_utils_link_files=${am_utils_link_files}libamu/${am_utils_mount_style_file}:conf/mount/mount_${ac_cv_style_mount}.c" "
dnl XXX: remove the next two lines after porting to autoconf-2.14
am_utils_link_files_src=${am_utils_link_files_src}conf/mount/mount_${ac_cv_style_mount}.c" "
am_utils_link_files_dst=${am_utils_link_files_dst}libamu/${am_utils_mount_style_file}" "

# append mount utilities object to LIBOBJS for automatic compilation
LIBOBJS="$LIBOBJS mountutil.o"
])
dnl ======================================================================
