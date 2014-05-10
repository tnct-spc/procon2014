buildtype := release

all :
	cd movement; buildtype=$(buildtype) make
	cd position; buildtype=$(buildtype) make

clean :
	cd movemant; buildtype=$(buildtype) make clean
	cd position; buildtype=$(buildtype) make clean

# vim:set tabstop=4 noexpendtab
