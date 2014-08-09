include ../makefile.in

OUTDIR :=../bin/$(buildtype)
TARGET := server

SRCS := src/http.cpp
PROG := $(OUTDIR)/$(TARGET)
OBJS := $(SRCS:src/%.cpp=obj/$(buildtype)/%.o)
DEPS := $(SRCS:src/%.cpp=obj/$(buildtype)/%.d)

CXXFLAGS +=
CPPFLAGS +=
LDFLAGS  +=
LIBS      = -lpthread -lboost_system -lboost_thread -lcppnetlib-uri -lcppnetlib-client-connections -lssl -lcrypto

.PHONY: all clean distclean

all: $(PROG)

-include $(DEPS)

$(PROG) : $(OBJS)
	@if [ ! -e `dirname $@` ]; then mkdir -p `dirname $@`; fi
	$(CXX) -o $@ $(OBJS) $(LIBS) $(LDFLAGS)

obj/$(buildtype)/%.o : src/%.cpp
	@if [ ! -e `dirname $@` ]; then mkdir -p `dirname $@`; fi
	$(CXX) $(CXXFLAGS) $(CPPFLAGS) $(DEFS) -o $@ -c -MMD -MP -MF $(@:%.o=%.d) $<

clean:
	rm -rf $(OUTDIR)

distclean:
	rm -rf ../bin

# vim:set tabstop=4 noexpandtab
