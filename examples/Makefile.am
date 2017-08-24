local_examples = \
 fetch \
 store \
 delete \
 prkeys

noinst_PROGRAMS = $(local_examples)
fetch_SOURCES = fetch.c common.c
fetch_CPPFLAGS = -I$(top_srcdir)/include
fetch_LDADD = $(top_srcdir)/libdbm/libdbm.la
store_SOURCES = store.c common.c
store_CPPFLAGS = -I$(top_srcdir)/include
store_LDADD = $(top_srcdir)/libdbm/libdbm.la
prkeys_SOURCES = prkeys.c common.c
prkeys_CPPFLAGS = -I$(top_srcdir)/include
prkeys_LDADD = $(top_srcdir)/libdbm/libdbm.la
delete_SOURCES = delete.c common.c
delete_CPPFLAGS = -I$(top_srcdir)/include
delete_LDADD = $(top_srcdir)/libdbm/libdbm.la
