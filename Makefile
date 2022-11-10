#Compilation variables
CC = gcc
CFLAGS = -Wall -std=c11 -MMD -Wextra
LDLIBS = -lreadline

#Executable name
TARGET = slash
TEST_TARGET = test/test_slash


#Build directory
BUILD_DIR = build
SRC_DIR = src
TEST_DIR = test

#Source files
SOURCES = $(filter-out $(SRC_DIR)/$(TARGET).c, $(wildcard $(SRC_DIR)/*.c))
TEST_SOURCES = $(wildcard $(TEST_DIR)/*.c)

#Binaries names
OBJECTS =  $(SOURCES:%.c=$(BUILD_DIR)/%.o)
TEST_OBJECTS = 	$(TEST_SOURCES:%.c=$(BUILD_DIR)/%.o)
TARGET_OBJECT = $(BUILD_DIR)/$(SRC_DIR)/$(TARGET).o

#Binaries dependencies
DEPENDENCIES = $(OBJECT:%.o=%.d) $(TEST_OBJECTS:%.o=%.d) $(TARGET_OBJECT:%.o:%.d)

-include $(DEPENDENCIES)


#JOBS
all: $(TARGET)

run: $(TARGET)
	@./$(TARGET)

test: $(TEST_TARGET)
	@./$(TEST_TARGET)

$(TARGET):  $(TARGET_OBJECT) $(OBJECTS)
	@$(CC) $(CFLAGS) $^ -o $(TARGET)  $(LDLIBS)

$(BUILD_DIR)/$(TARGET).o: $(OBJECTS)
	@$(CC) $(CFLAGS) $^ -o $(TARGET)  $(LDLIBS)

$(TEST_TARGET): $(TEST_OBJECTS) $(OBJECTS)
	@$(CC) $(CFLAGS) -o $(TEST_TARGET) $^ 
	
$(BUILD_DIR)/%.o: %.c
	@mkdir -p $(@D) 
	@$(CC) $(CFLAGS) -o $@ -c $< 

clean:
	@rm -rf $(BUILD_DIR) 
	@rm -f $(TARGET) $(TEST_TARGET)

