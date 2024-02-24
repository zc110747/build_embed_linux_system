#variable
execute ?=target
include=
cflags+=-O1 -lm
cc ?= gcc

objects = main.o \
		dir_api.o \
		process_api.o

all : $(execute)

%.o : %.c                                        
	$(cc) $(include) $(cflags) -c $< -o $@

$(execute):$(objects)
	$(cc) -o $(execute) $(objects) $(cflags)
	rm -rf $(objects)

clean:
	rm -rf $(execute)
	rm -rf $(objects)