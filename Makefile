bin/static/wave.o: lib/wave.cpp
	$(CXX) lib/wave.cpp -c -o bin/static/wave.o -O3 -Wall -Wextra -std=c++17 -Iinclude

bin/static/propagator.o: lib/propagator.cpp
	$(CXX) lib/propagator.cpp -c -o bin/static/propagator.o -O3 -Wall -Wextra -std=c++17 -Iinclude

bin/static/wfc.o: lib/wfc.cpp
	$(CXX) lib/wfc.cpp -c -o bin/static/wfc.o -O3 -Wall -Wextra -std=c++17 -Iinclude


static: results bin/static/wave.o bin/static/propagator.o bin/static/wfc.o
	ar rcs bin/static/libfastwfc.a bin/static/wave.o bin/static/propagator.o bin/static/wfc	.o

debug:
	$(CXX) lib/main.cpp -O3 -o wfc -g -Wall -Wextra -std=c++17 -Iinclude

results:
	mkdir results

clean:
	rm bin/static/*
