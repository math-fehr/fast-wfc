all: results
	$(CXX) lib/main.cpp -O3 -o wfc -Wall -Wextra -Wno-unused-parameter -std=c++17 -Iinclude

debug:
	$(CXX) lib/main.cpp -O3 -o wfc -g -Wall -Wextra -std=c++17 -Iinclude

results:
	mkdir results

clean:
	rm wfc
