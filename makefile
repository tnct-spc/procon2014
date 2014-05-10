buildtype := release

all :
	cd resolver;  buildtype=$(buildtype) make
	cd evaluator; buildtype=$(buildtype) make

clean :
	cd resolver;  buildtype=$(buildtype) make clean
	cd evaluator; buildtype=$(buildtype) make clean

# vim:set tabstop=4 noexpendtab
