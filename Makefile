all:
	gcc server_main.c -std=c11 storage.c minini/minIni.c `pkg-config --cflags --libs glib-2.0` -lpthread -o server
	gcc client_main.c -std=c11 -lpthread -o client
