all:
	clang -Weverything -Wno-unreachable-code-return -g3 -o x11-focus-pointer.bin x11-focus-pointer.cpp -lX11
