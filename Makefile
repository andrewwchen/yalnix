#
#	Sample Makefile for Yalnix kernel and user programs.
#	
#	Prepared by Sean Smith and Adam Salem and various Yalnix developers
#	of years past...
#
#	Nov 30, 2022



# Where's your kernel source?
K_SRC_DIR = .

# What are the kernel c and include files?
K_SRCS = 
K_INCS = 

# Where's your user source?
U_SRC_DIR = ./test

# What are the user c and include files?
U_SRCS = 
U_INCS = 


#==========================================================
# you should not need to change anything below this line
#==========================================================

#make all will make all the kernel objects and user objects
ALL = $(KERNEL_ALL) $(USER_APPS)
KERNEL_ALL = yalnix


# Automatically generate the list of sources, objects, and includes for the kernek
KERNEL_SRCS = $(K_SRCS:%=$(K_SRC_DIR)/%)
KERNEL_OBJS = $(KERNEL_SRCS:%.c=%.o) 
KERNEL_INCS = $(K_INCS:%=$(K_SRC_DIR)/%) 


# Automatically generate the list of apps, sources, objects, and includes for your userland coden
USER_SRCS = $(U_SRCS:%=$(U_SRC_DIR)/%)
USER_OBJS = $(USER_SRCS:%.c=%.o)
USER_APPS = $(USER_SRCS:%.c=%)
USER_INCS = $(U_INCS:%=$(U_SRC_DIR)/%) 

#write to output program yalnix
YALNIX_OUTPUT = yalnix



#Use the gcc compiler for compiling and linking
CC = gcc

DDIR58 = $(YALNIX_FRAMEWORK)
LIBDIR = $(DDIR58)/lib
INCDIR = $(DDIR58)/include
ETCDIR = $(DDIR58)/etc
#LIBRARY_PATH = $(ETCDIR)/linking/lib

# any extra loading flags...
LD_EXTRA = 

KERNEL_LIBS = $(LIBDIR)/libkernel.a $(LIBDIR)/libhardware.so

# the "kernel.x" argument tells the loader to use the memory layout in the kernel.x file..
KERNEL_LDFLAGS = $(LD_EXTRA) -L$(LIBDIR) -lkernel -lelf  -Wl,-T,$(ETCDIR)/kernel.x  -Wl,-R$(LIBDIR)  -lhardware
LINK_KERNEL = $(LINK.c)

#
#	These definitions affect how your Yalnix user programs are
#	compiled and linked.  Use these flags *only* when linking a
#	Yalnix user program.
#

USER_LIBS = $(LIBDIR)/libuser.a
ASFLAGS = -D__ASM__
CPPFLAGS=  -D_FILE_OFFSET_BITS=64 -m32 -fno-builtin -I. -I$(INCDIR) -g -DLINUX -fno-stack-protector


##########################
#Targets for different makes
# all: make all changed components (default)
# clean: remove all output (.o files, temp files, LOG files, TRACE, and yalnix)
# count: count and give info on source files
# list: list all c files and header files in current directory
# kill: close tty windows.  Useful if program crashes without closing tty windows.
# $(KERNEL_ALL): compile and link kernel files
# $(USER_ALL): compile and link user files
# %.o: %.c: rules for setting up dependencies.  Don't use this directly
# %: %.o: rules for setting up dependencies.  Don't use this directly

all: $(ALL)	

clean:
	rm -f *.o *~ TTYLOG* TRACE $(YALNIX_OUTPUT) $(USER_APPS) $(KERNEL_OBJS) $(USER_OBJS) core.* ~/core

count:
	wc $(KERNEL_SRCS) $(USER_SRCS)

list:
	ls -l *.c *.h

kill:
	killall yalnixtty yalnixnet yalnix

no-core:
	rm -f core.*

$(KERNEL_ALL): $(KERNEL_OBJS) $(KERNEL_LIBS) $(KERNEL_INCS)
	$(LINK_KERNEL) -o $@ $(KERNEL_OBJS) $(KERNEL_LDFLAGS)

$(USER_APPS): $(USER_OBJS) $(USER_INCS)
	$(ETCDIR)/yuserbuild.sh $@ $(DDIR58) $@.o










