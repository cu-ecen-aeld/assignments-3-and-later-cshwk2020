SRC := writer.c
#TARGET = ../finder-app/writer
#TARGET2 = ../../actions-runner-hw2/finder-app/writer
TARGET = writer
OBJS := $(SRC:.c=.o)
# make CROSS_COMPILE=aarch64_be-none-linux-gnu- clean
# make CROSS_COMPILE=aarch64_be-none-linux-gnu- all
# aarch64_be-none-linux-gnu-readelf -a writer | grep "Shared library"
# aarch64_be-none-linux-gnu-readelf -a writer | grep "program interpreter"

CROSS_COMPILE ?=
 
ifdef CROSS_COMPILE
    CC := $(CROSS_COMPILE)gcc
endif

all: $(TARGET)

$(TARGET) : $(OBJS)
	$(CC) $(CFLAGS) $(INCLUDES) $(OBJS) -o $(TARGET) $(LDFLAGS)

clean:
	-rm -f *.o $(TARGET) *.elf *.map

