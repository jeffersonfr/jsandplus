all:
	g++ main.cpp -o main -std=c++17 `pkg-config --cflags --libs jcanvas`

clean:
	rm -rf main
