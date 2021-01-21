CC=g++
CFLAG=-Wall -Wno-deprecated-declarations -O2
OPT=

OS=$(shell uname)
ifeq ($(OS), Darwin)    # Mac
LFLAG=-pthread -framework OpenGL -framework GLUT
else                    # Linux
LFLAG=-pthread -lglut -lGLU -lGL
endif

SRC=$(wildcard *.cc)
HDR=$(wildcard *.h)
OBJ=$(SRC:.cc=.o)
EXE=mandelbrot
SID=

.PHONY: tar clean

$(EXE): $(OBJ)
	$(CC) $(CFLAG) $(OPT) -o $@ $(OBJ) $(LFLAG)

%.o: %.cc $(HDR)
	$(CC) $(CFLAG) $(OPT) -o $@ -c $<

tar: $(SRC) $(HDR)
	@read -p "Enter your 10-digit student ID: " sid; \
	make clean; rm -f $$sid.tar; cd ../; \
	tar cvf $$sid.tar pthread/; mv $$sid.tar pthread/

clean:
	rm -f $(OBJ) $(EXE)

