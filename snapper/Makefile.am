#
# Makefile.am for snapper/snapper
#

AM_CXXFLAGS = -D_FILE_OFFSET_BITS=64 -I/usr/include/libxml2

lib_LTLIBRARIES = libsnapper.la

libsnapper_la_SOURCES =					\
	Factory.cc		Factory.h		\
	Snapper.cc		Snapper.h		\
	Snapshot.cc		Snapshot.h		\
	Comparison.cc		Comparison.h		\
	Filesystem.cc		Filesystem.h		\
	File.cc			File.h			\
	XmlFile.cc		XmlFile.h		\
	Enum.cc			Enum.h			\
	AppUtil.cc		AppUtil.h		\
	Log.cc			Log.h			\
	Compare.cc		Compare.h		\
	SystemCmd.cc		SystemCmd.h		\
	AsciiFile.cc		AsciiFile.h		\
	Regex.cc		Regex.h			\
	Exception.h					\
	SnapperTmpl.h					\
	SnapperTypes.h					\
	SnapperDefines.h

libsnapper_la_LDFLAGS = -version-info @LIBVERSION_INFO@
libsnapper_la_LIBADD = @LIBBLOCXX@ -lxml2 -lz -lm

pkgincludedir = $(includedir)/snapper

pkginclude_HEADERS =					\
	Factory.h					\
	Snapper.h					\
	Snapshot.h					\
	File.h						\
	Comparison.h					\
	Exception.h

