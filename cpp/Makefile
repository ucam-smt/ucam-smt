default:
	for file in `ls */Makefile`; do ( cd `dirname $$file`; make all; make install; ) ; done;
runtests:
	tests/bin/main.gtest
