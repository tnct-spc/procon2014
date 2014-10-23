include ../makefile.in

OUTDIR :=../bin/$(buildtype)
TARGET := pcserver

SRCS := $(wildcard src/*.cpp)
PROG := $(OUTDIR)/$(TARGET)
OBJS := $(SRCS:src/%.cpp=obj/$(buildtype)/%.o)
DEPS := $(SRCS:src/%.cpp=obj/$(buildtype)/%.d)

CXXFLAGS += -march=native -I./include
CPPFLAGS +=
LDFLAGS  +=
LIBS     += -lpthread -l_mongoose

DEST = /usr/local/bin/
RESDEST = /usr/share/pcserver/

.PHONY: all clean distclean install

all: $(PROG)

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
	rm -rf $(OUTDIR) obj

distclean:
	rm -rf ../bin

install:
	mkdir -p $(RESDEST)
	cp -ru res problem_set $(RESDEST)
	install $(PROG) $(DEST)

# vim:set tabstop=4 noexpandtab

