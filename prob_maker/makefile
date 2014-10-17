include ../makefile.in

OUTDIR := ../bin/$(buildtype)
TARGET = prob_maker

SRCS := src/main.cpp
PROG := $(OUTDIR)/$(TARGET)
OBJS := $(SRCS:src/%.cpp=obj/$(buildtype)/%.o)
DEPS := $(SRCS:src/%.cpp=obj/$(buildtype)/%.d)

CXXFLAGS +=
CPPFLAGS +=
LDFLAGS  +=
LIBS     += -lopencv_core -lopencv_imgproc -lopencv_highgui -lboost_program_options

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
