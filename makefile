CC = clang++
TARGET = procon
CXXFLAGS = -std=c++11
LIB = -lboost_system -lboost_filesystem$

./bin/$(TARGET) : ./bin/object/main.o ./bin/object/algorithm.o ./bin/object/splitter.o
	$(CC) $(LIB) -o $@ $^
	@echo "file output to ./bin/$(TARGET)"

./bin/object/main.o : ./main.cpp directory
	$(CC) $(CXXFLAGS) -c -o $@ $<

./bin/object/algorithm.o : ./algorithm.cpp directory
	$(CC) $(CXXFLAGS) -c -o $@ $<

./bin/object/splitter.o : ./splitter.cpp directory
	$(CC) $(CXXFLAGS) -c -o $@ $<

directory : 
	@mkdir -p ./bin/object

# vim:set tabstop=4 noexpendtab
