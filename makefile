CC = gcc
CFLAGS = -g -Wall -Wshadow

MASTER_EXE = master
MASTER_OBJ = master.o
MASTER_DEPS = master.c

SLAVE_EXE = slave
SLAVE_OBJ = slave.o
SLAVE_DEPS = slave.c

target: $(MASTER_EXE) $(SLAVE_EXE)

%.o: %.c
		$(CC) $(CFLAGS) -c -o $@ $^

$(MASTER_EXE): $(MASTER_OBJ)
		$(CC) $(CFLAGS) -o $(MASTER_EXE) $(MASTER_OBJ) -lm

$(SLAVE_EXE): $(SLAVE_OBJ)
		$(CC) $(CFLAGS) -o $(SLAVE_EXE) $(SLAVE_OBJ) -lm

.PHONY: clean

clean:
		-rm master slave *.o