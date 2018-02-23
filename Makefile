results:
	mkdir results

all: results
	g++ src/main.cpp -O3 -o wfc

debug:
	g++ src/main.cpp -o wfc -g

clean:
	rm wfc
