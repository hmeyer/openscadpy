rubyscript {
  HEADERS += src/rubyscript.h
  SOURCES += src/rubyscript.cc
  LIBS += -lrice -lruby1.8
  INCLUDEPATH += $$system( "ruby -e 'puts $:'" )
}
