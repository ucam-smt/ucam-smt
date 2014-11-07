default:
	make clean; for file in `ls */Makefile`; do ( cd `dirname $$file`; make all; make install; ) ; done;
clean:
	for file in `ls */Makefile`; do ( cd `dirname $$file`; make clean; ) ; done;
uninstall:
	rm -rf ../bin
