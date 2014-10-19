buildtype := release

.PHONY: all clean test

all :
	cd clammbon     ; buildtype=$(buildtype) $(MAKE)
	cd resolver     ; buildtype=$(buildtype) $(MAKE)
	cd eval_movement; buildtype=$(buildtype) $(MAKE)
	cd eval_position; buildtype=$(buildtype) $(MAKE)
	cd eval_server  ; buildtype=$(buildtype) $(MAKE)
	cd test         ; buildtype=$(buildtype) $(MAKE)
	cd prob_maker   ; buildtype=$(buildtype) $(MAKE)
	cd move_tester  ; buildtype=$(buildtype) $(MAKE)

clean :
	cd clammbon     ; buildtype=$(buildtype) $(MAKE) clean
	cd resolver     ; buildtype=$(buildtype) $(MAKE) clean
	cd eval_movement; buildtype=$(buildtype) $(MAKE) clean
	cd eval_position; buildtype=$(buildtype) $(MAKE) clean
	cd eval_server  ; buildtype=$(buildtype) $(MAKE) clean
	cd test         ; buildtype=$(buildtype) $(MAKE) clean
	cd prob_maker   ; buildtype=$(buildtype) $(MAKE) clean
	cd move_tester  ; buildtype=$(buildtype) $(MAKE) clean

test :
	cd clammbon     ; buildtype=$(buildtype) $(MAKE)
	cd test         ; buildtype=$(buildtype) $(MAKE)
	./bin/$(buildtype)/test

# vim:set tabstop=4 noexpendtab
