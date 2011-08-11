all:
	$(MAKE) -C src
	$(MAKE) -C runtime/tests

clean:
	$(MAKE) clean -C src
	$(MAKE) clean -C runtime/tests
		
test:
	cd runtime && \
	gnome-terminal -e "./cgl-render left" && \
	gnome-terminal -e "./cgl-render center" && \
	gnome-terminal -e "./cgl-render right" && \
	sleep 1 && \
	gnome-terminal -e "./cgl-capture tests/row/row"
	
debugrender:
	cd runtime && \
	gdb -ex run -quiet --args ./cgl-render left
