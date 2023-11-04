#-------------------------------------------------------------------------------
# variable define region.
# This region include compile tools, file, flags, include-path for the 
# program compile.
#-------------------------------------------------------------------------------
# build output file
Excutable ?= target
AR-Out ?=

# include build file information
Objects-C ?=
Objects-CPP ?=
Library ?=
IncludePath ?=

# compile flags
CFlags += -lm -lpthread
CPPFlags += -lm -lpthread

# compiler tools
CC ?= gcc
CCPLUS ?= g++
AR ?= ar

#-------------------------------------------------------------------------------
# script define region.
# This region include compile 
#-------------------------------------------------------------------------------
all : $(Excutable)

%.o : %.c                                           
	$(CC) $(IncludePath) $(CFlags) -c $< -o $@

%.o : %.cpp  
	$(CCPLUS) $(IncludePath) $(CPPFlags) -c $< -o $@

$(Excutable):$(Objects-C) $(Objects-CPP)
	$(shell if [ $(AR-Out) ]; then \
		$(AR) -cr $(AR-Out) $(Objects-C) $(Objects-CPP); \
	elif [ $(Objects-CPP) ] || [ $(Library) ]; then \
		$(CCPLUS) -o $(Excutable) $(Objects-C) $(Objects-CPP) $(Library) $(CFlags); \
	else \
		$(CC) -o $(Excutable) $(Objects-C) $(Library) $(CFlags); \
	fi)

clean:
	rm -rf $(Excutable) $(AR-Out)
	rm -rf $(Objects-C) $(Objects-CPP)