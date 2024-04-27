CC=gcc
FLAGS=-Wall -Werror
OBJS=mtll.o main.o
TARGET=mtll

all: $(TARGET)

build: $(TARGET) 

$(TARGET): $(OBJS)
	$(CC) $(FLAGS) $^ -o $@

%.o: %.c
	$(CC) -c $(FLAGS) $< -o $@

tests: $(TARGET)
	echo "Make my tests!"

.PHONY:
run_tests: tests
	echo "Run my tests!"

.PHONY:
clean:
	rm -f $(OBJS)
	rm -f $(TARGET)