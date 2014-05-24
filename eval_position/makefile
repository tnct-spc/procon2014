CXX ?= clang++

buildtype := release
OUTDIR := ../bin/$(buildtype)

TARGET = eval_position
SRCS := src/main.cpp
PROG := $(OUTDIR)/$(TARGET)
OBJS := $(SRCS:src/%.cpp=obj/$(buildtype)/%.o)
DEPS := $(SRCS:src/%.cpp=obj/$(buildtype)/%.d)
LIBS :=

CXXFLAGS += -std=c++11
CPPFLAGS += -I../clammbon/include
LDFLAGS  += -L../lib
ifeq ($(buildtype),release)
	CXXFLAGS += -O3
	LIBS += -lclammbon
else ifeq ($(buildtype),debug)
	CXXFLAGS += -O0 -g
	LIBS += -lclammbond
else
	$(error buildtype must be release, debug, profile or coverage)
endif

.PHONY: all clean distclean

all : $(PROG)

-include $(DEPS)

clammbon :
	cd ../clammbon; buildtype=$(buildtype) make

$(PROG) : $(OBJS) clammbon
	@if [ ! -e `dirname $@` ]; then mkdir -p `dirname $@`; fi
	$(CXX) $(LDFLAGS) -o $@ $(OBJS) $(LIBS)

obj/$(buildtype)/%.o : src/%.cpp
	@if [ ! -e `dirname $@` ]; then mkdir -p `dirname $@`; fi
	$(CXX) $(CXXFLAGS) $(CPPFLAGS) $(DEFS) -o $@ -c -MMD -MP -MF $(@:%.o=%.d) $<

clean:
	rm -rf $(OUTDIR)

distclean:
	rm -rf ../bin

# vim:set tabstop=4 noexpendtab
