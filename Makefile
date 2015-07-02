all:
	g++ main.cpp -o main -g -ggdb `pkg-config --cflags --libs jlibcpp`
