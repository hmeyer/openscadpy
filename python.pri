python {
	DEFINES += ENABLE_PYTHON BOOST_PYTHON_STATIC_LIB
	HEADERS += src/pythonscripting.h
	SOURCES += src/pythonscripting.cc
	INCLUDEPATH += /usr/include/python2.7/
	LIBS += -lpython2.7
  	win32 {
     		LIBS += -llibboost_python-vc90-mt-s-1_46_1
  	} else {
     		LIBS += -lboost_python-py26
  	}
}
