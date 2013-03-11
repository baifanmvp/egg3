gcc -legg3 ./eggadd.c -o eggadd `pkg-config glib-2.0 --cflags --libs` -O0 -g3 -Wall

gcc -legg3 ./eggdelete.c -o eggdelete `pkg-config glib-2.0 --cflags --libs` -O0 -g3 -Wall
 gcc -legg3 ./eggsearch.c -o eggsearch `pkg-config glib-2.0 --cflags --libs`  -O0 -g3 -Wall
#gcc -legg2 ./eggsearchloop.c -o eggsearchloop `pkg-config glib-2.0 --cflags --libs`  -O0 -g3 -Wall