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
lab4c_tls: lab4c_tls.c
	if uname -r | grep -q "edison" ; \
	then \
		gcc -g -Wall lab4c_tls.c -o lab4c_tls -lmraa -lm -lssl -lcrypto -std=c11 ; \
	else \
		gcc -g -Wall lab4c_tls.c -o lab4c_tls -DDUMMY -lm -lssl -lcrypto ; \
	fi
clean:
	rm -f lab4c_tcp lab4c_tls lab4c-404768745.tar.gz
dist: lab4c_tcp.c lab4c_tls.c Makefile README
	tar -czf lab4c-404768745.tar.gz lab4c_tcp.c lab4c_tls.c Makefile README
