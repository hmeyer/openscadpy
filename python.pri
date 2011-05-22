python {
	DEFINES += ENABLE_PYTHON BOOST_PYTHON_STATIC_LIB
	HEADERS += src/pythonscripting.h
	SOURCES += src/pythonscripting.cc
  	win32 {
     		LIBS += -llibboost_python-vc90-mt-s-1_46_1
		LIBS += -lpython2.7
  	} else {
		PYVERSION = $$system(python -c \"import sys;v=sys.version_info;print str(v[0])+\'.\'+str(v[1])\")
     		LIBS += -lboost_python
		LIBS += -lpython$$PYVERSION
		INCLUDEPATH += /usr/include/python$$PYVERSION/
  	}
}
