all: results
	$(CXX) main.cpp -O3 -o wfc -Wall -Wextra -Wno-unused-parameter -std=c++17 -I../include

debug:
	$(CXX) main.cpp -O3 -o wfc -g -Wall -Wextra -std=c++17 -I../include

results:
	mkdir results

clean:
	rm wfc
