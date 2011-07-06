all:
	$(MAKE) -C src
	$(MAKE) -C runtime/tests

clean:
	$(MAKE) clean -C src
	$(MAKE) clean -C runtime/tests
		
test:
	cd runtime && \
	gnome-terminal -e " gdb -ex run -quiet --args ./cgl-render left" && \
	gnome-terminal -e " gdb -ex run -quiet --args ./cgl-render right" && \
	sleep 1 && \
	gnome-terminal -e "./cgl-capture tests/lesson05/lesson05"
