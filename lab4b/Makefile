#	Name: Jeffrey Xu
#	Email: jeffreyhxu@gmail.com
#	ID: 404768745
all: lab4b

lab4b: lab4b.c
	if uname -r | grep -q "edison" ; \
	then \
		gcc -g -Wall lab4b.c -o lab4b -lmraa -lm ; \
	else \
		gcc -g -Wall lab4b.c -o lab4b -DDUMMY -lm ; \
	fi
check: lab4b

clean:

dist:
