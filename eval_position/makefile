CXX ?= clang++

buildtype := release
OUTDIR := ../bin/$(buildtype)

TARGET = eval_position
SRCS := src/main.cpp
PROG := $(OUTDIR)/$(TARGET)
OBJS := $(SRCS:src/%.cpp=obj/%.o)
DEPS := $(SRCS:src/%.cpp=obj/%.d)
LIBS :=

CXXFLAGS += -std=c++11
CPPFLAGS +=
LDFLAGS  +=
ifeq ($(buildtype),release)
	CXXFLAGS += -O3
else ifeq ($(buildtype),debug)
	CXXFLAGS += -O0 -g
else
	$(error buildtype must be release, debug, profile or coverage)
endif

.PHONY: all clean distclean

all : $(PROG)

-include $(DEPS)

$(PROG) : $(OBJS)
	@if [ ! -e `dirname $@` ]; then mkdir -p `dirname $@`; fi
	$(CXX) $(LDFLAGS) -o $@ $^ $(LIBS)

obj/%.o : src/%.cpp
	@if [ ! -e `dirname $@` ]; then mkdir -p `dirname $@`; fi
	$(CXX) $(CXXFLAGS) $(CPPFLAGS) $(DEFS) -o $@ -c -MMD -MP -MF $(@:%.o=%.d) $<

clean:
	rm -rf $(OUTDIR)

distclean:
	rm -rf ../bin

# vim:set tabstop=4 noexpendtab
