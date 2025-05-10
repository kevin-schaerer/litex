include ../include/generated/variables.mak
include $(SOC_DIRECTORY)/software/common.mak

OBJECTS =  \
	crc32.o    \
	system.o   \
	memtest.o  \
	uart.o     \
	isr.o

all: libbase.a

libbase.a: $(OBJECTS)
	$(AR) crs libbase.a $(OBJECTS)

# pull in dependency info for *existing* .o files
-include $(OBJECTS:.o=.d)

%.o: $(LIBBASE_DIRECTORY)/%.c
	$(compile)

.PHONY: all clean

clean:
	$(RM) $(OBJECTS)
	$(RM) libbase.a .*~ *~
