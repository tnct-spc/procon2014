include ../makefile.in

OUTDIR := ../lib
ifeq ($(buildtype),release)
	TARGET = libclammbon.a
else ifeq ($(buildtype),debug)
	TARGET = libclammbond.a
endif

SRCS := src/splitter.cpp src/ppm_reader.cpp src/algorithm.cpp src/gui.cpp src/sort_algorithm/adjacent.cpp src/sort_algorithm/compare.cpp src/sort_algorithm/genetic.cpp src/sort_algorithm/yrange2.cpp
PROG := $(OUTDIR)/$(TARGET)
OBJS := $(SRCS:src/%.cpp=obj/$(buildtype)/%.o)
DEPS := $(SRCS:src/%.cpp=obj/$(buildtype)/%.d)

CXXFLAGS +=
CPPFLAGS += -Iinclude

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
