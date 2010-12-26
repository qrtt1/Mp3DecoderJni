EXTRA=-Iinclude  -Llib 
EXTRA_MPG123=-lmpg123
EXTRA_JNI=-I/opt/java/include/ -I/opt/java/include/linux

ALL:
	gcc -g main.c $(EXTRA) $(EXTRA_MPG123)

DEMO:
	gcc -g mpg123_to_wav.c $(EXTRA) $(EXTRA_MPG123)

JNI: 
	gcc -o libdecoder.so -shared \
		-Wl,-soname,libdecoder.so  \
		qty_player_Mp3Decoder.c $(EXTRA_JNI) $(EXTRA) -static -lc lib/libmpg123.a


#gcc mpg123_to_wav.c -I include -Llib -lsndfile -lmpg123 

