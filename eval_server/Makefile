include ../makefile.in

OUTDIR :=../bin/$(buildtype)
TARGET := server

SRCS := src/answer.cpp src/problem.cpp src/server.cpp src/tools.cpp src/http-mongoose.cpp
PROG := $(OUTDIR)/$(TARGET)
OBJS := $(SRCS:src/%.cpp=obj/$(buildtype)/%.o)
DEPS := $(SRCS:src/%.cpp=obj/$(buildtype)/%.d)

TEST_SRCS := src/test.cpp src/tools.cpp
TEST_TARGET := server_test
TEST_PROG := $(OUTDIR)/$(TEST_TARGET)
TEST_OBJS := $(TEST_SRCS:src/%.cpp=obj/$(buildtype)/%.o)

CXXFLAGS += -march=native -ffast-math
CPPFLAGS +=
LDFLAGS  +=
LIBS     += -lpthread -l_mongoose

.PHONY: all clean distclean

all: $(PROG)

-include $(DEPS)

clammbon :
	cd ../clammbon; buildtype=$(buildtype) make

$(PROG) : $(OBJS) clammbon
	@if [ ! -e `dirname $@` ]; then mkdir -p `dirname $@`; fi
	$(CXX) -o $@ $(OBJS) $(LIBS) $(LDFLAGS)

obj/$(buildtype)/%.o : src/%.cpp
	@if [ ! -e `dirname $@` ]; then mkdir -p `dirname $@`; fi
	$(CXX) $(CXXFLAGS) $(CPPFLAGS) $(DEFS) -o $@ -c -MMD -MP -MF $(@:%.o=%.d) $<

test: $(TEST_PROG)

$(TEST_PROG) : $(TEST_OBJS)
	@if [ ! -e `dirname $@` ]; then mkdir -p `dirname $@`; fi
	$(CXX) -o $@ $(TEST_OBJS) $(LIBS) $(LDFLAGS)

clean:
	rm -rf $(OUTDIR) obj

distclean:
	rm -rf ../bin

# vim:set tabstop=4 noexpandtab

