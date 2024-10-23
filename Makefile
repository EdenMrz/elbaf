all: elbaf

clean:
	rm -rf elbaf elbaf.o

elbaf: elbaf.o
	g++ -o elbaf elbaf.o

elbaf.o: elbaf.cc
	g++ -c elbaf.cc
