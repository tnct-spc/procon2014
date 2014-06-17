include ../makefile.in

OUTDIR := ../bin/$(buildtype)
TARGET = procon

SRCS := src/main.cpp
PROG := $(OUTDIR)/$(TARGET)
OBJS := $(SRCS:src/%.cpp=obj/$(buildtype)/%.o)
DEPS := $(SRCS:src/%.cpp=obj/$(buildtype)/%.d)

CXXFLAGS +=
CPPFLAGS +=
LDFLAGS  +=
LIBS     +=

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
	rm -rf $(OUTDIR) ./obj

distclean:
	rm -rf ../bin ./obj

# vim:set tabstop=4 noexpendtab
