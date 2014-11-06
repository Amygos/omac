# Note: plugindir is set in configure

##############################################################################
# TODO: change libgstsrtpenc.la to something else, e.g. libmysomething.la     #
##############################################################################
plugin_LTLIBRARIES = libgstsrtpenc.la libgstsrtpdec.la

##############################################################################
# TODO: for the next set of variables, name the prefix if you named the .la, #
#  e.g. libmysomething.la => libmysomething_la_SOURCES                       #
#                            libmysomething_la_CFLAGS                        #
#                            libmysomething_la_LIBADD                        #
#                            libmysomething_la_LDFLAGS                       #
##############################################################################

# sources used to compile this plug-in
libgstsrtpenc_la_SOURCES = gstsrtpenc.c gstsrtpenc.h gstsrtp.c gstsrtp.h
libgstsrtpdec_la_SOURCES = gstsrtpdec.c gstsrtpdec.h gstsrtp.c gstsrtp.h

# compiler and linker flags used to compile this plugin, set in configure.ac
libgstsrtpenc_la_CFLAGS = $(GST_CFLAGS)
libgstsrtpenc_la_LIBADD = $(GST_LIBS)
libgstsrtpenc_la_LDFLAGS = $(GST_PLUGIN_LDFLAGS)
libgstsrtpenc_la_LIBTOOLFLAGS = --tag=disable-static

# compiler and linker flags used to compile this plugin, set in configure.ac
libgstsrtpdec_la_CFLAGS = $(GST_CFLAGS)
libgstsrtpdec_la_LIBADD = $(GST_LIBS)
libgstsrtpdec_la_LDFLAGS = $(GST_PLUGIN_LDFLAGS)
libgstsrtpdec_la_LIBTOOLFLAGS = --tag=disable-static


# headers we need but don't want installed
noinst_HEADERS = gstsrtpenc.h
noinst_HEADERS = gstsrtpdec.h
