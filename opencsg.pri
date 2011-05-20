opencsg {
  DEFINES += ENABLE_OPENCSG
  CONFIG += glew
  include(glew.pri)

  HEADERS += src/render-opencsg.h
  SOURCES += src/render-opencsg.cc

  OPENCSG_DIR = opencsg/OpenCSG-1.3.1
  INCLUDEPATH += $$OPENCSG_DIR/include
  LIBS += -L$$OPENCSG_DIR/lib
  PRE_TARGETDEPS += $$OPENCSG_DIR/lib/libopencsg.a
  LIBS += -Wl,-Bstatic -lopencsg -Wl,-Bdynamic

	opencsg.target = "$$OPENCSG_DIR/lib/libopencsg.a"
	opencsg.commands = cd opencsg;$(MAKE)

  QMAKE_EXTRA_TARGETS += opencsg
  QMAKE_CLEAN += -r opencsg/OpenCSG-1.3.1 opencsg/OpenCSG-1.3.1.tar.gz
}
