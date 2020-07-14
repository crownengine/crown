.PHONY: all
all: gresources.xml
	glib-compile-resources gresources.xml --target=gresources.c --generate

.PHONY: clean
clean:
	-@rm gresources.c

.PHONY: rebuild
rebuild: clean all
