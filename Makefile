all:
	$(MAKE) -C src

clean:
	$(MAKE) clean -C src
	
		
run-app:
	#./runtime/cgl 640 480 0 0 &
	cd runtime && LD_PRELOAD=./libcgl.so ./lesson05/lesson05


run-svr:
	./runtime/cgl 640 480 0 0 
