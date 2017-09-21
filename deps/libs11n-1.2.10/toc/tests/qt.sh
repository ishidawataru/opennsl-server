# toc_run_description = looking for Qt $1
# toc_begin_help =
#  Calls toc_export for the following:
#  - QTINCDIR   directory containing Qt headers
#  - QTLIBDIR   directory containing Qt libraries
#  - QTBINDIR   directory containing Qt tools (moc, uic, etc.)
#  - LQT        set to "-lqt"
#  - UIC        /path/to/uic
#  - MOC        /path/to/moc
#  - QMAKE      /path/to/qmake
#  - QT_INCLUDES    -I/path/to/Qt/includes
#  - QT_LDFLAGS    -L/path/to/Qt/libs
#  Also, QTINCDIR, QTLIBDIR, and QTBINDIR are set in the environment.  Note
#  that this intentionally does not set QTDIR, to keep you from using
#  $QTDIR/bin etc. instead of the QTBINDIR which might have been specified
#  by the user.
#
#  If you need a specific version of Qt, set both of the following variables
#  before running this test:
#    qt_required_version - human-readable version number like "3.1.x"
#    qt_required_version_glob - version glob like "3.1.*"
#  Optionally, pass the version as $1 and glob as $2 (be sure to quote the
#  glob!).
# = toc_end_help

qt_required_version_version=${1-${qt_required_version_version-'3.x'}}
qt_required_version_glob=${2-${qt_required_version_glob-'3.*'}}

qt_check_dir ()
{
	local var=$1
	local dir=$2
	local contents=$3
	if test -z "$dir"; then
		cat<<EOF
$var environment variable not set, and --with-${var}=... not passed!
Use one or the other to point to your Qt $contents, or QTDIR or
--with-QTDIR=... to point to the top of your Qt installation.
EOF
		return 1
	fi
	if test -d $dir; then
		#  life is tolerable
		return 0
	else
        cat<<EOF
Directory $dir does not exist!
Set the $var environment variable, or pass --with-${var}=... to
point to your Qt $contents, or QTDIR or --with-QTDIR=... to point
to the top of your Qt installation.
EOF
		return 1
	fi
}

#  Use --with-QTDIR=... if we got it; otherwise default to $QTDIR.
QTDIR=${configure_with_QTDIR-$QTDIR}
toc_export configure_with_qt=0
toc_export HAVE_QT=0

if test -z "${QTDIR}"; then
    echo "error: either set QTDIR or pass in --with-QTDIR=/path/to/qt"
    return 1
fi

#  QTINCDIR/QTLIBDIR/QTBINDIR can be specified independently, in the
#  environment or on the command line, or we can guess them from QTDIR.
QTINCDIR=${configure_with_QTINCDIR-${QTINCDIR-${QTDIR+$QTDIR/include}}}
QTLIBDIR=${configure_with_QTLIBDIR-${QTLIBDIR-${QTDIR+$QTDIR/lib}}}
QTBINDIR=${configure_with_QTBINDIR-${QTBINDIR-${QTDIR+$QTDIR/bin}}}

qt_check_dir QTINCDIR "$QTINCDIR" headers || return 1
qt_check_dir QTLIBDIR "$QTLIBDIR" libraries || return 1
qt_check_dir QTBINDIR "$QTBINDIR" executables || return 1

if test -n "$qt_required_version"; then
	#  This has to be libqt*.so, not libqt.so, because in some cases you
	#  have only libqt-mt.so.x.y.z, and libqt.so is a symlink to that, and
	#  there are no libqt.so.x.y.z symlinks.  (At least that's the way it
	#  is with Qt 3.1.1 built from source on my RedHat 8.0 box.)
	ls $QTLIBDIR/libqt*.so.${qt_required_version_glob} > /dev/null 2>&1 || {
		echo "Qt $qt_required_version (libqt*.so.$qt_required_version_glob) not found in $QTLIBDIR!"
		return 1
	}
        echo "Found Qt $qt_required_version at ${QTLIBDIR}"
fi


export QTDIR
export QTINCDIR
export QTLIBDIR
export QTBINDIR
toc_export QTDIR=$QTDIR
toc_export QTINCDIR=$QTINCDIR
toc_export QTLIBDIR=$QTLIBDIR
toc_export QTBINDIR=$QTBINDIR
toc_export LQT=-lqt
toc_export UIC=${UIC:-$QTBINDIR/uic}
toc_export MOC=${MOC:-$QTBINDIR/moc}
toc_export QMAKE=${QMAKE:-$QTBINDIR/qmake}

toc_export QT_INCLUDES="-I${QTINCDIR}"
toc_export QT_LDFLAGS="-L${QTLIBDIR}"

configure_with_qt=1
toc_export configure_with_qt=1
toc_export HAVE_QT=1
return 0
