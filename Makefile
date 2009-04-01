all:
	$(MAKE) -C src

clean:
	$(MAKE) clean -C src
	
		
run-app:
	#./runtime/cgl &
	cd runtime && LD_PRELOAD=./libcgl.so ./lesson05/lesson05


run-q3:
	cd runtime && LD_PRELOAD=./libcgl.so openarena
	#./runtime/cgl &
