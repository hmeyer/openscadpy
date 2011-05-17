python {
	DEFINES += ENABLE_PYTHON
	HEADERS += src/pythonscripting.h
	SOURCES += src/pythonscripting.cc
	INCLUDEPATH += /usr/include/python2.7/
	LIBS += -lboost_python-py26 -lpython2.7
}
