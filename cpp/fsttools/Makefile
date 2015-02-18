include ../../Makefile.inc

CC=g++ -Wfatal-errors -rdynamic # -fno-builtin-malloc -fno-builtin-calloc -fno-builtin-realloc -fno-builtin-free -fno-omit-frame-pointer
DBG=-ggdb3 -gdwarf-2 
PRO=-pg

LIBS=-lfst -lpthread -lm -ldl $(BOOST_LOG_LIB1) $(BOOST_LOG_LIB2) -lboost_regex -lboost_program_options -lboost_iostreams -lboost_serialization -lboost_filesystem -lboost_system -lkenlm -lboost_thread -pthread -lz -lrt
INCLUDE=-I./ -Iinclude -I../include -I../fsttools/include
OINCLUDE=-I$(OPENFST_INCLUDE) -I$(BOOST_INCLUDE) -I./ -Iinclude -I../include -I$(KENLM_DIR)
OLIBDIRS=-L$(OPENFST_LIB) -L$(BOOST_LIB) -L../../bin 
LIBSSO=-lfst -lm -ldl 
OINCLUDESO=-I$(OPENFST_INCLUDE) -I./ -Iinclude -I../include
OLIBDIRSSO=-L$(OPENFST_LIB)
BIN_DIR=bin/

LD_LIBRARY_PATH=$(OPENFST_LIB):$(BOOST_LIB):$(GTEST_LIB)

ifdef NPLM_LIB
CC+=-DWITH_NPLM
LIBS+=-lneuralLM -fopenmp
OINCLUDE+=-I$(EIGEN_INCLUDE) -I$(NPLM_INCLUDE)
OLIBDIRS+=-L$(NPLM_LIB)
LD_LIBRARY_PATH:=$(LD_LIBRARY_PATH):$(NPLM_LIB)
endif


default:
	make usage


usage:
	@echo -e "Usage: \n All binaries have a main cpp file with the following name: [binaryname].main.cpp. A library will have a [libraryname].so.cpp main file. Options are:\n\
	make [binaryname].std          : std compilation\n\
	make [binaryname].O2           : compilation with O2 optimization\n\
	make [binaryname].O3           : compilation with O3 optimization\n\
	make [binaryname].sta          : static compilation\n\
	make [binaryname].sta.O2       : static compilation with O2 optimization\n\
	make [binaryname].sta.O3       : static compilation with O3 optimizaiton\n\
	make [binaryname].sta.gdb      : static compilation with extra flags for gdb symbols\n\
	make [binaryname].debug0       : compilation with extra flags for gdb symbols, binary only prints intermediate lattice files\n\
	make [binaryname].debug        : compilation with extra flags for gdb symbols, binary prints intermediate lattice files, also verbosity increased with dbg messages\n\
	make [binaryname].sta.debug0   : static compilation with extra flags for gdb symbols, binary only prints intermediate lattice files\n\
	make [binaryname].sta.debug    : static compilation with extra flags for gdb symbols, binary prints intermediate lattice files, also verbosity increased with dbg messages\n\
	make [binaryname].pro          : compile for profiling\n\
	make [binaryname].pro.O2       : compile for profiling, including O2 optimizations\n\
	make [libraryname].so	       : compile dynamic library\n\
	make all		       : makes all the binaries and shared libraries within this directory leaving them on $(PWD)/bin \n\
	make install                   : copies all tools to ../../bin/\n"

install:
	mkdir -p ../../bin
	cp $(BIN_DIR)/* ../../bin/
all:
	make -j $(NUMPROC) `for k in \`ls *.main.cpp *.so.cpp\`; do target=\`echo $$k | sed -e 's:.main.cpp:.$(TGTBINMK):g;s:.so.cpp:.so:g'\`; echo $$target ; done 2> /dev/null `

clean:
	rm -rf bin obj

%.sta.gdb:
	mkdir -p bin	
	$(CC) -static -lstdc++ $(DBG) $(INCLUDE) $(OINCLUDE) $(OLIBDIRS) $*.main.cpp -o $(BIN_DIR)/$*.sta.gdb.bin -fPIC $(LIBS)

%.sta: 
	mkdir -p bin	
	$(CC) -static -lstdc++ $(INCLUDE) $(OINCLUDE) $(OLIBDIRS) $*.main.cpp -o $(BIN_DIR)/$*.sta.bin -fPIC $(LIBS)

%.sta.O2: 
	mkdir -p bin	
	$(CC) -static -lstdc++ -O2 $(INCLUDE) $(OINCLUDE) $(OLIBDIRS) $*.main.cpp -o $(BIN_DIR)/$*.sta.O2.bin -fPIC $(LIBS)

%.sta.O3: 
	mkdir -p bin	
	$(CC) -static -lstdc++ -O3 $(INCLUDE) $(OINCLUDE) $(OLIBDIRS) $*.main.cpp -o $(BIN_DIR)/$*.sta.O3.bin -fPIC $(LIBS)

%.std: 
	mkdir -p bin
	cat ../../scripts/TEMPLATE | sed -e 's|%%LD_LIBRARY_PATH%%|${LD_LIBRARY_PATH}|g' > $(BIN_DIR)/$*.std ; chmod +x $(BIN_DIR)/$*.std
	$(CC) -lstdc++ $(INCLUDE) $(OINCLUDE) $(OLIBDIRS) $*.main.cpp -o $(BIN_DIR)/$*.std.bin -fPIC $(LIBS)

%.O2: 
	mkdir -p bin	
	cat ../../scripts/TEMPLATE | sed -e 's|%%LD_LIBRARY_PATH%%|${LD_LIBRARY_PATH}|g' > $(BIN_DIR)/$*.O2 ;	chmod +x $(BIN_DIR)$*.O2
	$(CC) -lstdc++ -O2 $(INCLUDE) $(OINCLUDE) $(OLIBDIRS) $*.main.cpp -o $(BIN_DIR)/$*.O2.bin -fPIC $(LIBS)

%.O3: 
	mkdir -p bin	
	cat ../../scripts/TEMPLATE | sed -e 's|%%LD_LIBRARY_PATH%%|${LD_LIBRARY_PATH}|g' > $(BIN_DIR)/$*.O3 ; chmod +x $(BIN_DIR)/$*.O3
	$(CC) -lstdc++ -O3 $(INCLUDE) $(OINCLUDE) $(OLIBDIRS) $*.main.cpp -o $(BIN_DIR)/$*.O3.bin -fPIC $(LIBS)

%.strip:
	make $*
	strip $(BIN_DIR)/$*

%.so: 
	mkdir -p bin
	$(CC) -shared -lstdc++ $(OPT) $(INCLUDE) $(OINCLUDESO) $(OLIBDIRSSO) $*.so.cpp -o $(BIN_DIR)/$*.so -fPIC $(LIBSSO) 

%.sta.debug: 
	mkdir -p bin
	$(CC) -static -lstdc++ $(DBG) -DPRINTDEBUG1 $(INCLUDE) $(OINCLUDE) $(OLIBDIRS) $*.main.cpp -o $(BIN_DIR)/$*.sta.debug.bin -fPIC $(LIBS)

%.sta.debug0: 
	mkdir -p bin
	$(CC) -static -lstdc++ $(DBG) -DPRINTDEBUG $(INCLUDE) $(OINCLUDE) $(OLIBDIRS) $*.main.cpp -o $(BIN_DIR)/$*.sta.debug0.bin -fPIC $(LIBS)

%.debug: 
	mkdir -p bin
	cat ../../scripts/TEMPLATE | sed -e 's|%%LD_LIBRARY_PATH%%|${LD_LIBRARY_PATH}|g' > $(BIN_DIR)/$*.debug ; chmod +x $(BIN_DIR)/$*.debug
	$(CC) -lstdc++ $(DBG) -DPRINTDEBUG1 $(INCLUDE) $(OINCLUDE) $(OLIBDIRS) $*.main.cpp -o $(BIN_DIR)/$*.debug.bin -fPIC $(LIBS)

%.debug0: 
	mkdir -p bin
	cat ../../scripts/TEMPLATE | sed -e 's|%%LD_LIBRARY_PATH%%|${LD_LIBRARY_PATH}|g' > $(BIN_DIR)/$*.debug0 ; chmod +x $(BIN_DIR)/$*.debug0
	$(CC) -lstdc++ $(DBG) -DPRINTDEBUG $(INCLUDE) $(OINCLUDE) $(OLIBDIRS) $*.main.cpp -o $(BIN_DIR)/$*.debug0.bin -fPIC $(LIBS)

%.pro: 
	mkdir -p bin
	$(CC) -lstdc++ $(PRO) $(INCLUDE) $(OINCLUDE) $(OLIBDIRS) $*.main.cpp -o $(BIN_DIR)/$*.pro.bin -fPIC $(LIBS)

%.pro.O2:
	mkdir -p bin
	$(CC) -lstdc++ $(PRO) $(OPT) $(INCLUDE) $(OINCLUDE) $(OLIBDIRS) $*.main.cpp -o $(BIN_DIR)/$*.pro.O2.bin -fPIC $(LIBS)
%:
	mkdir -p bin
	make $*.$(TGTBINMK)
