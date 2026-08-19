CC=gcc
CFLAGS=-std=c99 -Wall -Wextra -Wpedantic

ft: ft.c
	$(CC) $^ -o $@ $(CLFAGS)
