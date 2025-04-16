#for test makefile, run command need order.
#make BUILD=s
#make BUILD=d
#make BUILD=e

#-------------------------------------------------------------------------------
# define the build output, only one below can choose.
# library example(build out static library): AR-Out = static_lib.a
# Executable example(build out Executable): Excutable = target
#-------------------------------------------------------------------------------
BUILD ?=e

ifeq ($(BUILD),e) 
Excutable=target
Static-LIB=libtest
Dynamic-LIB=dynamictest
endif

ifeq ($(BUILD),s)
Static-LIB=libtest
Static-LIB-SRC=static_test.o
endif

ifeq ($(BUILD),d)
Dynamic-LIB=dynamictest
Dynamic-LIB-SRC=dynamic_test.o
endif

#-------------------------------------------------------------------------------
# define files to compile, c files follow Objects-C, 
# cpp file follow Objects-CPP. if with out cpp, not set
# C file example: Objects-C = main.o 
# CPP file example: Objects-CPP = application.o
#-------------------------------------------------------------------------------
Objects-C=main.o 
Objects-CPP=application.o

#-------------------------------------------------------------------------------
# include c/c++ header path
#-------------------------------------------------------------------------------
IncludePath =

#-------------------------------------------------------------------------------
# define CFlags and CPPFlags, if not define, use default.
# CFLags example: CFlags = -O1
# CPPFlags example: CPPFlags = -std=c++11
#-------------------------------------------------------------------------------
CFlags = -O0
CPPFlags = -std=c++17

#-------------------------------------------------------------------------------
# library when compile link, if not, must empty
# example: Library = static-lib.a
#-------------------------------------------------------------------------------

#-------------------------------------------------------------------------------
# Compile Tools default used gcc/g++/ar, for g++, need change compiler
# c compiler example: CC = arm-linux-gnueabihf-gcc
# cpp compiler example: CCPLUS = arm-linux-gnueabihf-g++
# ar compiler example": AR = arm-linux-gnueabihf-ar
#-------------------------------------------------------------------------------

# Including this file allows access to the Build System.
include buildtool.mk