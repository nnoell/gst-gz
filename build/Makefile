## Common Compiler Flags

CC = cc
CFLAGS = -ggdb3 -Wall -fpic -O3 \
         -Wextra -Wformat=2 -Werror -Wfatal-errors -Wwrite-strings -Winit-self -Wcast-align -Wpointer-arith \
         -Wstrict-aliasing=2 -Wmissing-include-dirs -Wno-unused-parameter -Wuninitialized -Wold-style-definition \
         -Wstrict-prototypes -Wno-missing-braces -Winline -Wundef -Wno-missing-field-initializers -Wswitch-default \
         -Wswitch-enum -Wstrict-overflow=5 -Wfloat-equal -Wnested-externs -Wshadow -Wunreachable-code -Wredundant-decls
LIBS = -pthread -lgobject-2.0 -lglib-2.0 -lz -lbz2
INCLUDES = -I/usr/include/glib-2.0 -I/usr/lib/glib-2.0/include

## Use gstreamer 1.0 API
GST_API_CFLAGS = -I/usr/include/gstreamer-1.0
GST_API_LFLAGS = -lgstbase-1.0 -lgstreamer-1.0

## Use gstreamer 0.10 API
#GST_API_CFLAGS = -DUSE_GSTREAMER_010 -I/usr/include/gstreamer-0.10 -I/usr/include/libxml2
#GST_API_LFLAGS = -lgstbase-0.10 -lgstreamer-0.10

## ---------------------------------------------------------------------------------------------------------------------

all: ../src/libgstgz.so

../src/libgstgz.so: ../src/gstgzdec.o ../src/gstgz.o
	@${CC} -shared -o ../src/libgstgz.so ../src/gstgzdec.o ../src/gstgz.o ${GST_API_LFLAGS} ${LIBS}
	@echo "Linking $@"

../src/gstgzdec.o: ../src/gstgzdec.c
	@${CC} ${CFLAGS} ${GST_API_CFLAGS} ${INCLUDES} -c $< -o ../src/gstgzdec.o
	@echo "Compiling $<"

../src/gstgz.o: ../src/gstgz.c
	@${CC} ${CFLAGS} ${GST_API_CFLAGS} ${INCLUDES} -c $< -o ../src/gstgz.o
	@echo "Compiling $<"

clean:
	@rm -f ../src/libgstgz.so ../src/gstgzdec.o ../src/gstgz.o
