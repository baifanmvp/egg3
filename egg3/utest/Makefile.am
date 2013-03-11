AUTOMAKE_OPTIONS = foreign

bin_PROGRAMS = eggUtest #eggadd eggmodify eggdelete eggquery

eggUtest_SOURCES = eggUtest.cpp\
                    eggItfTest.cpp\
                    eggFileTest.cpp\
                    eggConfigTest.cpp\
                    eggHttpTest.cpp\
                    eggViewStreamTest.cpp\
                    eggIdViewTest.cpp\
                    eggBTreeTest.cpp\
                    eggIdTableTest.cpp \
                    eggPathTest.cpp \
                    eggWeightTest.cpp \
		    eggThreadPoolTest.cpp\
		    eggWeightTest.h

eggUtest_CPPFLAGS = -I../src/ ${AM_CPPFLAGS}  ${glib_CFLAGS}
eggUtest_LDADD = $(glib_LIBS) ${AM_LDFLAGS} -lcwsplugin -lcppunit -legg3



eggUtest_LDFLAGS = $(AM_LDFLAGS) 
eggUtest_CXXFLAGS = $(AM_CXXFLAGS) 

 # eggadd_SOURCES=eggadd.c
 # eggadd_CPPFLAGS = -I../src/ ${AM_CPPFLAGS}  ${glib_CFLAGS}
 # eggadd_LDADD = $(glib_LIBS) -legg3 ${AM_LDFLAGS} -lcwsplugin  


# eggadd_SOURCES=eggadd.c
# eggadd_CPPFLAGS = -I../src/ ${AM_CPPFLAGS}  ${glib_CFLAGS}
# eggadd_LDADD = $(glib_LIBS) -legg3 ${AM_LDFLAGS} -lcwsplugin  

# eggmodify_SOURCES=eggmodify.c
# eggmodify_CPPFLAGS = -I../src/ ${AM_CPPFLAGS}  ${glib_CFLAGS}
# eggmodify_LDADD = $(glib_LIBS) -legg3 ${AM_LDFLAGS} -lcwsplugin  

# eggdelete_SOURCES=eggdelete.c
# eggdelete_CPPFLAGS = -I../src/ ${AM_CPPFLAGS}  ${glib_CFLAGS}
# eggdelete_LDADD = $(glib_LIBS) -legg3 ${AM_LDFLAGS} -lcwsplugin  

# eggquery_SOURCES=eggquery.c
# eggquery_CPPFLAGS = -I../src/ ${AM_CPPFLAGS}  ${glib_CFLAGS}
# eggquery_LDADD = $(glib_LIBS) -legg3 ${AM_LDFLAGS} -lcwsplugin  
