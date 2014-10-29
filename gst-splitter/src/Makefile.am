# Note: plugindir is set in configure

##############################################################################
# TODO: change libgstsplitter.la to something else, e.g. libmysomething.la     #
##############################################################################
plugin_LTLIBRARIES = libgstsplitter.la

##############################################################################
# TODO: for the next set of variables, name the prefix if you named the .la, #
#  e.g. libmysomething.la => libmysomething_la_SOURCES                       #
#                            libmysomething_la_CFLAGS                        #
#                            libmysomething_la_LIBADD                        #
#                            libmysomething_la_LDFLAGS                       #
##############################################################################

# sources used to compile this plug-in
libgstsplitter_la_SOURCES = gstsplitter.c gstsplitter.h

# compiler and linker flags used to compile this plugin, set in configure.ac
libgstsplitter_la_CFLAGS = $(GST_CFLAGS)
libgstsplitter_la_LIBADD = $(GST_LIBS)
libgstsplitter_la_LDFLAGS = $(GST_PLUGIN_LDFLAGS)
libgstsplitter_la_LIBTOOLFLAGS = --tag=disable-static

# headers we need but don't want installed
noinst_HEADERS = gstsplitter.h
