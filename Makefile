ALL:
	gcc -g main.c -I include -Llib -lsndfile -lmpg123 

DEMO:
	gcc -g mpg123_to_wav.c -I include -Llib -lsndfile -lmpg123 

#gcc mpg123_to_wav.c -I include -Llib -lsndfile -lmpg123 

