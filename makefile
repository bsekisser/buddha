CFLAGS = -g -Wall -O2 $(INCLUDE)
CFLAGS += -MD -MP

TARGET := vm

SRC_DIR := source
OBJ_DIR := build-$(shell $(CC) -dumpmachine)

INCLUDE = -Iinclude -I../include
INCLUDE += -I../../lib_dtime

VPATH = ./include:../include:../../include:../lib_dtime

SRCS = $(wildcard $(SRC_DIR)/*.c)
OBJS = $(patsubst $(SRC_DIR)/%.c, $(OBJ_DIR)/%.o, $(SRCS))
DEPS = $(patsubst $(SRC_DIR)/%.c, $(OBJ_DIR)/%.d, $(SRCS))



all: $(OBJ_DIR) $(OBJ_DIR)/$(OBJS) target



target: $(OBJ_DIR)/$(TARGET)



$(OBJ_DIR):
	-mkdir -p $(OBJ_DIR)

$(OBJ_DIR)/$(TARGET) : $(OBJS)
	$(CC) $(LFLAGS) $^ -o $@ $(LDFLAGS)

$(OBJS) : $(OBJ_DIR)/%.o : $(SRC_DIR)/%.c
	$(CC) -c $(CFLAGS) $(CPPFLAGS) $< -o $@



clean:
	-rm -rf $(OBJ_DIR)/*.d $(OBJ_DIR)/*.o $(OBJ_DIR)/$(TARGET) *.i *.s *.o *.d



-include $(BUILD)/*.d
