include ../../Makefile.inc

CC=g++ -Wfatal-errors -rdynamic $( $(GTEST_DIR)/scripts/gtest-config --cxxflags --ldflags --libs ) # -DUSE_FDSTREAM_HPP -DUSE_BOOSTLOG 

OPT=-O2
DBG=-ggdb3 -gdwarf-2 
LIBS=-lfst -lm -lpthread -ldl -lgtest $(BOOST_LOG_LIB1) $(BOOST_LOG_LIB2) -lboost_program_options -lboost_regex -lboost_iostreams -lboost_serialization -lboost_filesystem -lboost_system -lboost_thread -pthread -lkenlm -lz -lrt
#PRINTDEBUG=-DPRINTDEBUG1

OINCLUDE=-I$(OPENFST_INCLUDE) -I$(BOOST_INCLUDE) -I$(KENLM_DIR) -I./ -Iinclude -I../include  -I../hifst/include -I../fsttools/include  -I$(GTEST_INCLUDE) 
OLIBDIRS=-L$(OPENFST_LIB)  -L$(BOOST_LIB) -L$(GTEST_LIB)  -L../../bin
BIN_DIR=bin/
OBJ_DIR=obj/

usage:
	@echo -e "Usage: \n All test binaries correspond to a gtest.cpp file with the following name: [binaryname].gtest.cpp. \
	Available options are:\n\
	make [binaryname].gtest.std ==> Use standard (dynamic) compilation\
	make [binaryname].gtest.sta ==> Use static compilation\
									\
	In addition, a single binary with all tests can be created in the following way:\
	make main.std\n\
	make main.sta\n"
default:
	make usage

clean:
	rm obj/*.o bin/*

install:
	cp bin/main.gtest.* ../../bin/	

obj/%.gtest.o:
	mkdir -p $(OBJ_DIR)
	$(CC) -c $(DBG) -DGMAINTEST $(PRINTDEBUG) -DGTEST_USE_OWN_TR1_TUPLE=0  $(DBG) $(INCLUDE) $(OINCLUDE) $(OLIBDIRS) $*.gtest.cpp  -o $(@) 

%.gtest.std: 
	$(CC) $(DBG) -lstdc++  $(PRINTDEBUG) -DGTEST_USE_OWN_TR1_TUPLE=0  $(DBG) $(INCLUDE) $(OINCLUDE) $(OLIBDIRS) $*.gtest.cpp  -o $(BIN_DIR)/$(@) -fPIC $(LIBS)
%.gtest.sta: 
	$(CC) $(DBG) -lstdc++ -static $(PRINTDEBUG) -DGTEST_USE_OWN_TR1_TUPLE=0  $(DBG) $(INCLUDE) $(OINCLUDE) $(OLIBDIRS) $*.gtest.cpp  -o $(BIN_DIR)/$(@) -fPIC $(LIBS)

obj/main.gtest.hifst.o: main.gtest.hifst.cpp *.gtest.cpp
	mkdir -p $(OBJ_DIR)
	$(CC) -c $(DBG)  -DGMAINTEST $(PRINTDEBUG) -DGTEST_USE_OWN_TR1_TUPLE=0  $(DBG) $(INCLUDE) $(OINCLUDE) $(OLIBDIRS) main.gtest.hifst.cpp  -o $(@)  

main.std: *.gtest.cpp
	mkdir -p $(BIN_DIR)
	rm -f $(OBJ_DIR)/*.o
	make -j  $(NUMPROC) `ls *.gtest.cpp | sed -e 's:.cpp:.o:g;s:^:obj/:g'` obj/main.gtest.hifst.o
	$(CC) $(DBG) -lstdc++  $(PRINTDEBUG) -DGMAINTEST -DGTEST_USE_OWN_TR1_TUPLE=0  $(DBG) $(INCLUDE) $(OINCLUDE) $(OLIBDIRS) $(OBJ_DIR)/*.o  -o $(BIN_DIR)/main.gtest.std -fPIC $(LIBS)

main.sta: *.gtest.cpp
	mkdir -p $(BIN_DIR)
	rm -f $(OBJ_DIR)/*.o
	make -j  $(NUMPROC) `ls *.gtest.cpp | sed -e 's:.cpp:.o:g;s:^:obj/:g'` obj/main.gtest.hifst.o
	$(CC) $(DBG) -static -lstdc++  $(PRINTDEBUG) -DGMAINTEST -DGTEST_USE_OWN_TR1_TUPLE=0  $(DBG) $(INCLUDE) $(OINCLUDE) $(OLIBDIRS) $(OBJ_DIR)/*.o  -o $(BIN_DIR)/main.gtest.sta -fPIC $(LIBS)


main.sta.%:
	make main.sta

main.std.%:
main.%:
	make main.std

allunittests.std:
	make -j $(NUMPROC) `ls *.gtest.cpp | sed -e 's:.cpp:.std:g'`

allunittests.sta:
	make -j $(NUMPROC) `ls *.gtest.cpp | sed -e 's:.cpp:.sta:g'`

all:
	make main.$(TGTBINMK)