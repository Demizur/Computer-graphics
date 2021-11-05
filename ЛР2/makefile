all: render

render: main.o tga.o model.o
	gcc main.o tga.o model.o -o render

main.o: main.c tga.h model.h
	gcc -c main.c tga.h model.h
    
tga.o:tga.c tga.h
	gcc -c tga.c tga.h

model.o:model.c model.h tga.h
	gcc -c model.c model.h tga.h

clean:
	rm -rf *.o render