all:
	$(MAKE) -C src

clean:
	$(MAKE) clean -C src
	
		
run-app:
	cd runtime && LD_PRELOAD=./libcgl.so ./lesson12/lesson12
	#./runtime/cgl &
