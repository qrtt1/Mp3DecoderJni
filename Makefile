EXTRA=-I/home/qrtt1/Downloads/mpg123-1.13.0/OUTPUT/include  -L/home/qrtt1/Downloads/mpg123-1.13.0/OUTPUT/lib 
EXTRA_MPG123=-lmpg123
#-Wl,-rpath=/home/qrtt1/Downloads/mpg123-1.13.0/OUTPUT/lib
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

