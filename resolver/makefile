include ../makefile.in

OUTDIR := ../bin/$(buildtype)
TARGET = procon

SRCS := src/main.cpp
PROG := $(OUTDIR)/$(TARGET)
OBJS := $(SRCS:src/%.cpp=obj/$(buildtype)/%.o)
DEPS := $(SRCS:src/%.cpp=obj/$(buildtype)/%.d)

CXXFLAGS +=
CPPFLAGS += -Iinclude
LDFLAGS  +=
LIBS     += -lboost_system -lboost_thread -lboost_coroutine -lboost_timer -lboost_program_options -lpthread -lcppnetlib-server-parsers -lcppnetlib-client-connections -lcppnetlib-uri -lcrypto -lssl

.PHONY: all clean distclean

all : $(PROG)

-include $(DEPS)

clammbon :
	cd ../clammbon; buildtype=$(buildtype) $(MAKE)

$(PROG) : $(OBJS) clammbon
	@if [ ! -e `dirname $@` ]; then mkdir -p `dirname $@`; fi
	$(CXX) -o $@ $(OBJS) $(LIBS) $(LDFLAGS)

obj/$(buildtype)/%.o : src/%.cpp
	@if [ ! -e `dirname $@` ]; then mkdir -p `dirname $@`; fi
	$(CXX) $(CXXFLAGS) $(CPPFLAGS) $(DEFS) -o $@ -c -MMD -MP -MF $(@:%.o=%.d) $<

clean:
	rm -rf $(OUTDIR) ./obj

distclean:
	rm -rf ../bin ./obj

# vim:set tabstop=4 noexpendtab
