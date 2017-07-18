binary=bin/strategy

run:
	if test ! -s $(binary); then make build; fi;
	$(binary)

build:
	mkdir -p bin
	g++ -O2 -fno-stack-limit -x c++ -std=c++11 src/main.cxx -o $(binary)
