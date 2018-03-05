results:
	mkdir results

all: results
	g++ src/main.cpp -O3 -o wfc -Wall -Wextra -std=c++17

debug:
	g++ src/main.cpp -O3 -o wfc -g -Wall -Wextra -std=c++17

clean:
	rm wfc
