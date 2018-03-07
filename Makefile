results:
	mkdir results

all: results
	$(CXX) src/main.cpp -O3 -o wfc -Wall -Wextra -Wno-unused-parameter -std=c++17

debug:
	$(CXX) src/main.cpp -O3 -o wfc -g -Wall -Wextra -std=c++17

clean:
	rm wfc
