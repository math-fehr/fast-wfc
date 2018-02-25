results:
	mkdir results

all: results
	g++ src/main.cpp -O3 -o wfc -Wall -Wextra

debug:
	g++ src/main.cpp -O3 -o wfc -g -Wall -Wextra

clean:
	rm wfc
