CC ?= gcc
FLAGS ?= -Wall -std=c11

TARGET ?= bit
TEST_TARGET ?= test/testpreview 
OBJECTS ?=  
TEST_OBJECTS ?= build/testlib.o build/testpreview.o
all: $(TARGET)

run: $(TARGET)
	@./$(TARGET)

test: $(TEST_TARGET)
	@./$(TEST_TARGET)

$(TARGET): $(TARGET) $(OBJECTS)
	@$(CC) $(FLAGS) -o $(TARGET) $^

$(TEST_TARGET): $(TEST_OBJECTS) $(OBJECTS)
	@$(CC) $(FLAGS) -o $(TEST_TARGET) $^ 
	
./build/%.o: ./test/%.c
	@$(CC) $(FLAGS) -o $@ -c $< 

clean: 
	@rm -f *.o $(TARGET) $(TEST_TARGET)

