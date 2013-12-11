
# define the C compiler to use
UNAME_S := $(shell uname -s)
  ifeq ($(UNAME_S),Linux)
	CC = g++-4.7
  endif
  ifeq ($(UNAME_S),Darwin)
	CC = g++
  endif

# define any compile-time flags
CFLAGS = -Wall -O2 -std=c++11  -DNDEBUG # -pg

# define any directories containing header files other than /usr/include
#
ifeq ($(UNAME_S),Linux)
    INCLUDES = -I./include -I/usr/lib/erlang/usr/include/
endif
ifeq ($(UNAME_S),Darwin)
    INCLUDES = -I./include -I/usr/local/lib/erlang/lib/erl_interface-3.7.13/include/
endif

# define library paths in addition to /usr/lib
#   if I wanted to include libraries not in /usr/lib I'd specify
#   their path using -Lpath, something like:
ifeq ($(UNAME_S),Linux)
    LFLAGS = -lerl_interface -lei -lnsl -pthread -lc
endif
ifeq ($(UNAME_S),Darwin)
    LFLAGS = -lc++  -lerl_interface -lei -lc
endif

# define any libraries to link into executable:
#   if I want to link in libraries (libx.so or libx.a) I use the -llibname 
#   option, something like (this will link in libmylib.so and libm.so:
  ifeq ($(UNAME_S),Linux)
    LIBS = -L/usr/lib/erlang/usr/lib#
  endif
  ifeq ($(UNAME_S),Darwin)
	LIBS = -L/usr/local/lib/erlang/lib/erl_interface-3.7.13/lib #
  endif

# define the C source files
SRCS = src/main.cc src/test_suite.cc src/tally.cc src/count_handlers.cc src/metric_handlers.cc src/event_array.cc src/event_metric_array.cc src/leaderboard.cc src/idx_assigner.cc  

# define the C object files 
#
# This uses Suffix Replacement within a macro:
#   $(name:string1=string2)
#         For each word in 'name' replace 'string1' with 'string2'
# Below we are replacing the suffix .c of all words in the macro SRCS
# with the .o suffix
#
OBJS = $(SRCS:.cc=.o)

# define the executable file 
MAIN = etally_srv

#
# The following part of the makefile is generic; it can be used to 
# build any executable just by changing the definitions above and by
# deleting dependencies appended to the file from 'make depend'
#

.PHONY: depend clean

all: $(MAIN)
	@echo  Compiling tally_srv complete 

$(MAIN): $(OBJS) 
	$(CC) $(CFLAGS) $(INCLUDES) -o $(MAIN) $(OBJS) $(LFLAGS) $(LIBS)

# this is a suffix replacement rule for building .o's from .c's
# it uses automatic variables $<: the name of the prerequisite of
# the rule(a .c file) and $@: the name of the target of the rule (a .o file) 
# (see the gnu make manual section about automatic variables)
.cc.o:
	$(CC) $(CFLAGS) $(INCLUDES) -c $<  -o $@

clean:
	$(RM) */*.o *~ $(MAIN)

depend: $(SRCS)
	makedepend $(INCLUDES) $^

# DO NOT DELETE THIS LINE -- make depend needs it
