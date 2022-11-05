#Compilation variables
CC = gcc
CFLAGS = -Wall -std=c11 -MMD

#Executable name
TARGET = bit
TEST_TARGET = test/test_slash

#Source files
SOURCES = $(wildcard src/*.c)
TEST_SOURCES = $(wildcard test/*.c)

#Build directory
BUILD_DIR = build

#Binaries names
OBJECTS =  $(SOURCES:%.c=$(BUILD_DIR)/%.o)
TEST_OBJECTS = 	$(TEST_SOURCES:%.c=$(BUILD_DIR)/%.o)

#Binaries dependencies
DEPENDENCIES = $(OBJECT:%.o=%.d) $(TEST_OBJECTS:%.o=%.d)

-include $(DEPENDENCIES)


#JOBS
all: $(TARGET)

run: $(TARGET)
	@./$(TARGET)

test: $(TEST_TARGET)
	@./$(TEST_TARGET)

$(TARGET): $(TARGET) $(OBJECTS)
	@$(CC) $(CFLAGS) -o $(TARGET) $^

$(TEST_TARGET): $(TEST_OBJECTS) $(OBJECTS)
	@$(CC) $(CFLAGS) -o $(TEST_TARGET) $^ 
	
$(BUILD_DIR)/%.o: %.c
	@mkdir -p $(@D) 
	@$(CC) $(CFLAGS) -o $@ -c $< 
clean:
	@rm -rf $(BUILD_DIR) 
	@rm -f $(TARGET) $(TEST_TARGET)

