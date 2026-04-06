CC     = gcc
PKG_CONFIG ?= pkg-config
UNAME_S := $(shell uname -s)

CFLAGS = -Wall -O2 -I. $(shell $(PKG_CONFIG) --cflags raylib)
LIBS   = $(shell $(PKG_CONFIG) --libs raylib) -lm
ifeq ($(UNAME_S),Linux)
LIBS += -ldl -lpthread
endif

TARGET = gun

SRCS = main.c \
       coords.c \
       src/algo/dda.c \
       src/algo/bresenham.c \
       src/algo/midcircle.c \
       src/ui/back_button.c \
       src/ui/cartesian.c \
       src/ui/audio_manager.c \
       src/ui/target.c \
       src/screens/about.c \
       src/screens/menu.c \
       src/screens/glock.c \
       src/screens/kar98k.c

OBJS = $(SRCS:.c=.o)

all: $(TARGET)

$(TARGET): $(OBJS)
	$(CC) $(CFLAGS) -o $(TARGET) $(OBJS) $(LIBS)

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -f $(TARGET) $(OBJS)

run: all
	LIBGL_ALWAYS_SOFTWARE=1 ./$(TARGET)

.PHONY: all clean run
