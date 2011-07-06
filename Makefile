all:
	$(MAKE) -C src
	$(MAKE) -C runtime/tests

clean:
	$(MAKE) clean -C src
	$(MAKE) clean -C runtime/tests
		
test:
	cd runtime && ./cgl-render testing
