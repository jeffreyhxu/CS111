#	Name: Jeffrey Xu
#	Email: jeffreyhxu@gmail.com
#	ID: 404768745
all: lab4c_tcp lab4c_tls

lab4c_tcp: lab4c_tcp.c
	if uname -r | grep -q "edison" ; \
	then \
		gcc -g -Wall lab4c_tcp.c -o lab4c_tcp -lmraa -lm -std=c11 ; \
	else \
		gcc -g -Wall lab4c_tcp.c -o lab4c_tcp -DDUMMY -lm ; \
	fi
lab4c_tls:

clean:

dist:
