all:
	gcc server_main.c storage.c minini/minIni.c `pkg-config --cflags --libs glib-2.0` -lpthread -o server
	gcc client_main.c -lpthread -o client
