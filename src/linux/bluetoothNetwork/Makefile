CC = gcc
TARGETS = rfcomm-client

.PHONY: all
all: $(TARGETS)

$(TARGETS): %:%.o
	$(CC) -lm -lwiringPi -lbluetooth -o $@ $^

.PHONY: clean
clean:
	rm *.o rfcomm-client
