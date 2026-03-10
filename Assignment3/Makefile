TARGET = myuserthreads
LIBS =  #List of external libraries required to link against

MODIFYVALUE_HEADERS = modify_value.h  
MODIFYVALUE_SRCS = modify_value.c  
MODIFYVALUE_CC_FLAGS = -Wall -std=gnu11 

MAIN_HEADERS = 
MAIN_SRCS = myuserthreads.c  
MAIN_CC_FLAGS = -O3 -Wall -std=gnu11 

OBJECTS = $(MAIN_SRCS:.c=.o) $(MODIFYVALUE_SRCS:.c=.o)

.PHONY: default all clean

all: $(TARGET)

modify_value: $(MODIFYVALUE_SRCS) $(MODIFYVALUE_HEADERS) 
	$(CC) $(MODIFYVALUE_CC_FLAGS) -c $< -o $@.o

main: $(MAIN_SRCS) $(MAIN_HEADERS) 
	$(CC) $(MAIN_CC_FLAGS) -c $< -o $@.o

$(TARGET): $(OBJECTS)
	$(CC) $(CC_FLAGS) $(OBJECTS) $(LIBS) -o $@

clean:
	-rm -f *.o
	-rm -f $(TARGET)
