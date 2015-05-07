CC = gcc
TARGETS = client server

.PHONY: all
all: $(TARGETS)

$(TARGETS): %:%.o
	$(CC) -lm -lwiringPi -o $@ $^

.PHONY: clean
clean:
	rm *.o client server
