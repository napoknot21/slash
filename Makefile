CC ?= gcc
FLAGS ?= -Wall -std=c11

TARGET ?= bit
TEST_TARGET ?= testpreview 
OBJECTS ?=  
TEST_OBJECTS ?= testlib.o testpreview.o
all: $(TARGET)

run: $(TARGET)
	@./$(TARGET)

test: $(TEST_TARGET)
	@./$(TEST_TARGET)

$(TARGET): $(TARGET) $(OBJECTS)
	@$(CC) $(FLAGS) -o $(TARGET) $^

$(TEST_TARGET): $(TEST_OBJECTS) $(OBJECTS)
	@$(CC) $(FLAGS) -o $(TEST_TARGET) $^ 
	
%.o: %.c
	@$(CC) $(FLAGS) -o $@ -c $< 

clean: 
	@rm -f *.o $(TARGET) $(TEST_TARGET)

