#-------------------------------------------------------------------------------
# variable define region.
# This region include compile tools, file, flags, include-path for the 
# program compile.
#-------------------------------------------------------------------------------
# build output file
Excutable ?=
Static-LIB ?=
Dynamic-LIB ?=

# include build file information
Objects-C ?=
Objects-CPP ?=
Library ?=
IncludePath ?=

# compile flags
CFlags += -lm -lpthread
CPPFlags += -lm -lpthread

# compiler tools
CC ?=gcc
CCPLUS ?=g++
AR ?=ar
OBJ = 

#-------------------------------------------------------------------------------
# script define region.
# This region include compile 
#-------------------------------------------------------------------------------
%.o : %.c                                           
	$(CC) $(IncludePath) $(CFlags) -c $< -o $@

%.o : %.cpp  
	$(CCPLUS) $(IncludePath) $(CPPFlags) -c $< -o $@

##############################################################################
## Check output
##############################################################################
ifeq ($(Excutable),)
ifneq ($(Static-LIB),)
ifneq ($(Dynamic-LIB),)
$(error only one library can build each times!)
endif
endif
endif

ifeq ($(CCPLUS),g++)
$(info used compiler:$(CCPLUS))
endif

ifeq ($(Excutable),)
##############################################################################
## compile static library
##############################################################################
ifneq ($(Static-LIB),)
all : $(Static-LIB)

#build static library
$(Static-LIB) : $(Static-LIB-SRC)
	$(AR) -cr $(Static-LIB).a $(Static-LIB-SRC)

clean:
	rm -rf $(Static-LIB).a $(Static-LIB-SRC)
endif

##############################################################################
## compile dynamic library
##############################################################################
ifneq ($(Dynamic-LIB),)
all : $(Dynamic-LIB)

#build dynamic library
$(Dynamic-LIB) : $(Dynamic-LIB-SRC)
	$(CCPLUS) $(Dynamic-LIB-SRC) -fPIC -shared -o lib$(Dynamic-LIB).so

clean:
	rm -rf lib$(Dynamic-LIB).so $(Dynamic-LIB-SRC)
endif

##############################################################################
## compile excutable
##############################################################################
else
all : $(Excutable)

OBJ += $(Objects-C)
OBJ += $(Objects-CPP)

#build for output
$(Excutable):$(OBJ)
	if [ $(Objects-CPP) ] || [ $(Library) ]; then \
		$(CCPLUS) -o $(Excutable) $(Objects-C) $(Objects-CPP) $(Library) $(Static-LIB).a -L. -l$(Dynamic-LIB) $(CFlags); \
	else \
		$(CC) -o $(Excutable) $(Objects-C) $(Library) $(AR-Out) $(CFlags); \
	fi

clean:
	rm -rf *.o
	rm -rf $(Excutable)
endif


