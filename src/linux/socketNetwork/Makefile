CC = gcc
TARGETS = clientMain

.PHONY: all
all: $(TARGETS)

$(TARGETS): %:%.o
	$(CC) -lm -lwiringPi -o $@ $^

.PHONY: clean
clean:
	rm *.o clientMain
