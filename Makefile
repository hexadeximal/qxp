all:
	gcc main.c -o qxp -Wall -Wextra -Wimplicit-fallthrough -pedantic -ggdb -fno-omit-frame-pointer \
	`xml2-config --cflags --libs` -lcurl \
