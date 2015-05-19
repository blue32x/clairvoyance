CC = gcc
TARGETS = simpleScan rfcomm-server rfcomm-client

.PHONY: all
all: $(TARGETS)

$(TARGETS): %:%.o
	$(CC) -lm -lwiringPi -lbluetooth -o $@ $^

.PHONY: clean
clean:
	rm *.o simpleScan rfcomm-server rfcomm-client
