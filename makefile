buildtype := release

all :
	cd clammbon     ; buildtype=$(buildtype) make
	cd resolver     ; buildtype=$(buildtype) make
	cd eval_movement; buildtype=$(buildtype) make
	cd eval_position; buildtype=$(buildtype) make

clean :
	cd clammbon     ; buildtype=$(buildtype) make clean
	cd resolver     ; buildtype=$(buildtype) make clean
	cd eval_movement; buildtype=$(buildtype) make clean
	cd eval_position; buildtype=$(buildtype) make clean

# vim:set tabstop=4 noexpendtab
