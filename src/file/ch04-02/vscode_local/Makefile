
EXCUTABLE ?= target

OBJECTS = main.o application/test_file.o

INCLUDE_PATH = 

#-g表示支持调试
CFLAGS += -O1 -lm -g

CC := gcc                                           

all : $(EXCUTABLE)

%.o : %.c                                           
	$(CC) $(INCLUDE_PATH) $(CFLAGS) -c $< -o $@

$(EXCUTABLE):$(OBJECTS)
	$(CC) -o $(EXCUTABLE) $(OBJECTS) $(CFLAGS)

clean:
	rm -rf $(EXCUTABLE)
	rm -rf $(OBJECTS)