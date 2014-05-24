CXX ?= clang++
AR := ar

buildtype := release
OUTDIR := ../lib

ifeq ($(buildtype),release)
	TARGET = libclammbon.a
else ifeq ($(buildtype),debug)
	TARGET = libclammbond.a
endif

SRCS := src/splitter.cpp src/ppm_reader.cpp src/pixel_sorter.cpp src/algorithm.cpp
PROG := $(OUTDIR)/$(TARGET)
OBJS := $(SRCS:src/%.cpp=obj/$(buildtype)/%.o)
DEPS := $(SRCS:src/%.cpp=obj/$(buildtype)/%.d)
LIBS :=

CXXFLAGS += -std=c++11
CPPFLAGS += -Iinclude
LDFLAGS  +=
ifeq ($(buildtype),release)
	CXXFLAGS += -O3
else ifeq ($(buildtype),debug)
	CXXFLAGS += -O0 -g -D_DEBUG
else
	$(error buildtype must be release, debug, profile or coverage)
endif

.PHONY: all clean distclean

all : $(PROG)

-include $(DEPS)

$(PROG) : $(OBJS)
	@if [ ! -e `dirname $@` ]; then mkdir -p `dirname $@`; fi
	$(AR) rs -o $@ $^

obj/$(buildtype)/%.o : src/%.cpp
	@if [ ! -e `dirname $@` ]; then mkdir -p `dirname $@`; fi
	$(CXX) $(CXXFLAGS) $(CPPFLAGS) $(DEFS) -o $@ -c -MMD -MP -MF $(@:%.o=%.d) $<

clean:
	rm -rf $(OUTDIR)/$(TARGET) ./obj

distclean:
	rm -rf ../lib ./obj

# vim:set tabstop=4 noexpendtab
