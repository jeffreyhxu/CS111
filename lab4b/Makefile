#	Name: Jeffrey Xu
#	Email: jeffreyhxu@gmail.com
#	ID: 404768745
all: lab4b

lab4b: lab4b.c
	if uname -r | grep -q "edison" ; \
	then \
		gcc -g -Wall lab4b.c -o lab4b -lmraa -lm -std=c11 ; \
	else \
		gcc -g -Wall lab4b.c -o lab4b -DDUMMY -lm ; \
	fi
check: lab4b
	if echo -e "SCALE=C\nPERIOD=2\nSTOP\nSTART\nOFF" | ./lab4b | grep -q -E "[0-9]{2}:[0-9]{2}:[0-9]{2} [0-9]+.[0-9]" ; \
	then \
		echo "Smoke test passed" ; \
	else \
		echo "Smoke test failed" ; \
	fi
clean:
	rm -f lab4b lab4b-404768745.tar.gz
dist: lab4b.c Makefile README
	tar -czf lab4b-404768745.tar.gz lab4b.c Makefile README
