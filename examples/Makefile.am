local_examples = \
 fetch \
 store

noinst_PROGRAMS = $(local_examples)
fetch_SOURCES = fetch.c common.c
fetch_CPPFLAGS = -I$(top_srcdir)/include
fetch_LDADD = $(top_srcdir)/libdbm/libdbm.la
store_SOURCES = store.c common.c
store_CPPFLAGS = -I$(top_srcdir)/include
store_LDADD = $(top_srcdir)/libdbm/libdbm.la
