all: OpenCSG-1.3.1/lib/libopencsg.a

clean:
	rm -rf OpenCSG-1.3.1/

OpenCSG-1.3.1/lib/libopencsg.a: OpenCSG-1.3.1/.modmak
	cd OpenCSG-1.3.1/src;$(MAKE)

OpenCSG-1.3.1/.modmak: OpenCSG-1.3.1
	awk 'NR==137{$$0="all: Makefile ../lib/$$(TARGET) $$(TARGETA)"}1' OpenCSG-1.3.1/src/Makefile >t
	awk 'NR==22{$$0="LIBS     = $$(SUBLIBS) -L/usr/X11R6/lib -lGLU -lGL"}1' t >t2
	mv t2 OpenCSG-1.3.1/src/Makefile
	rm t

OpenCSG-1.3.1: OpenCSG-1.3.1.tar.gz
	tar xzf OpenCSG-1.3.1.tar.gz

OpenCSG-1.3.1.tar.gz:
	wget http://www.opencsg.org/OpenCSG-1.3.1.tar.gz
