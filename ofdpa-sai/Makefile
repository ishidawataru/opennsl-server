all:
	gcc -I /usr/include/libnl3 -I /root -I /root/sonic-sairedis/SAI/inc -fPIC -shared sai.c -o libsai.so -lrpc_client
	mv libsai.so /lib/x86_64-linux-gnu

