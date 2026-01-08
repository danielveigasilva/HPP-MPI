TARGET = ./bin/HPP-MPI
SRC = HPP-MPI.c
CC = mpicc

all: $(TARGET)

$(TARGET): $(SRC) | bin
	$(CC) -o $(TARGET) $(SRC)
bin:
	mkdir -p bin
clean:
	rm -f $(TARGET)
