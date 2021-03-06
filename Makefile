#CC=gcc -m64
#CFLAGS=-g -Wall -std=c99 -O0
CC=icc
CFLAGS=-O3 -Wall -march=native -std=c99
INCLUDE=-I $(HOME)/include $(HDF5_C_INCLUDE) $(MPI_CFLAGS)
LIB=-L $(HOME)/lib -lgsl -lgslcblas $(HDF5_C_LIBS)

OBJS_AUTO=hash.o auto_counts.o read_hdf5.o main.o
OBJS_TEST_AUTO=hash.o auto_counts.o read_hdf5.o test_auto.o
OBJS_CROSS=hash.o cross_counts.o read_hdf5.o main_cross.o
OBJS_TEST_CROSS=hash.o cross_counts.o read_hdf5.o test_cross.o
EXEC_AUTO = auto
EXEC_CROSS = cross
EXEC_TEST_AUTO = test_auto
EXEC_TEST_CROSS = test_cross

default: auto cross test_auto test_cross

clean:
	rm *.o; rm $(EXEC_AUTO) $(EXEC_CROSS) $(EXEC_TEST_AUTO) $(EXEC_TEST_CROSS)

main.o: main.c
	$(CC) $(CFLAGS) $(INCLUDE) -c $< -o $@

main_cross.o: main_cross.c
	$(CC) $(CFLAGS) $(INCLUDE) -c $< -o $@

read_hdf5.o: read_hdf5.c
	$(CC) $(CFLAGS) $(INCLUDE) -c $< -o $@

auto_counts.o: auto_counts.c
	$(CC) $(CFLAGS) $(INCLUDE) -c $< -o $@

cross_counts.o: cross_counts.c
	$(CC) $(CFLAGS) $(INCLUDE) -c $< -o $@

hash.o: hash.c
	$(CC) $(CFLAGS) $(INCLUDE) -c $< -o $@

test_auto.o: test_auto.c
	$(CC) $(CFLAGS) $(INCLUDE) -c $< -o $@

test_cross.o: test_cross.c
	$(CC) $(CFLAGS) $(INCLUDE) -c $< -o $@

test_auto: $(OBJS_TEST_AUTO)
	$(CC) $(CFLAGS) $(OBJS_TEST_AUTO) $(LIB) -o $(EXEC_TEST_AUTO)

test_cross: $(OBJS_TEST_CROSS)
	$(CC) $(CFLAGS) $(OBJS_TEST_CROSS) $(LIB) -o $(EXEC_TEST_CROSS)

auto: $(OBJS_AUTO)
	$(CC) $(CFLAGS) $(OBJS_AUTO) $(LIB) -o $(EXEC_AUTO)

cross: $(OBJS_CROSS)
	$(CC) $(CFLAGS) $(OBJS_CROSS) $(LIB) -o $(EXEC_CROSS)
