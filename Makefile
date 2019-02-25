CC=aarch64-himix100-linux-gcc

default:
	$(CC) crypto.c sha204-core.c i2c-core.c -fPIC -shared -O0 -o libcrypto_d.so
	$(CC) main.c  -L . -lcrypto_d -O0 -o main_d

	$(CC) -O0 -o main_e main.c crypto.c sha204-core.c i2c-core.c 
	
	$(CC) -c crypto.c sha204-core.c i2c-core.c
	ar -r libcrypto_s.a crypto.o sha204-core.o i2c-core.o
	$(CC) main.c -static -L. -lcrypto_s -o main_s
clean:
	rm -f main_*[eds] *.o *.so *.a

