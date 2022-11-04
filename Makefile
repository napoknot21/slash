CC ?= gcc
FLAGS ?= -Wall -std=c11

TARGET ?= bit
TEST_TARGET ?= test/testpreview 
OBJECTS ?=  
TEST_OBJECTS ?= build/test/testlib.o build/test/testpreview.o
all: $(TARGET)

run: $(TARGET)
	@./$(TARGET)

test: $(TEST_TARGET)
	@./$(TEST_TARGET)

$(TARGET): $(TARGET) $(OBJECTS)
	@$(CC) $(FLAGS) -o $(TARGET) $^

$(TEST_TARGET): $(TEST_OBJECTS) $(OBJECTS)
	@$(CC) $(FLAGS) -o $(TEST_TARGET) $^ 
	
./build/test/%.o: ./test/%.c build build/test
	@$(CC) $(FLAGS) -o $@ -c $< 

./build/src/%.o: ./src/%.c build build/src
	@$(CC) $(FLAGS) -o $@ -c $<

clean: 
	@rm -rf ./build rm -f $(TARGET) $(TEST_TARGET)

build/src: build
	@mkdir build/src

build/test: build
	@mkdir build/test

build:
	@mkdir build 
