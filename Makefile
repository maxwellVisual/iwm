SRC_DIR=src/
BUILD_DIR=build/
GCC_FLAGS=-fPIC -lpng -O0

C_SOURCE=$(SRC_DIR)/libiwm.c $(SRC_DIR)/image.c
OBJ_SRC = $(BUILD_DIR)/obj/image.c.o $(BUILD_DIR)/obj/libiwm.c.o

.PHONY: all always clear iwm static_lib dynamic_lib

all: clean iwm static_lib dynamic_lib

$(BUILD_DIR)/obj/%.c.o: $(SRC_DIR)/%.c always objdir
	gcc $(GCC_FLAGS) -c -o $@ $<

objdir: always
	mkdir -p $(BUILD_DIR)/obj/

static_lib: clean $(BUILD_DIR)/libiwm.a
$(BUILD_DIR)/libiwm.a: always $(OBJ_SRC)
	ar -r $(BUILD_DIR)/libiwm.a $(BUILD_DIR)/obj/*

dynamic_lib: clean $(BUILD_DIR)/libiwm.so
$(BUILD_DIR)/libiwm.so: always
	gcc $(OBJ_SRC) -shared $(GCC_FLAGS) -o $(BUILD_DIR)/libiwm.so

iwm: clean $(BUILD_DIR)/iwm
$(BUILD_DIR)/iwm: always static_lib
	gcc $(OBJ_SRC) $(SRC_DIR)/main.c $(GCC_FLAGS) -o $(BUILD_DIR)/iwm -L$(BUILD_DIR) -liwm -g

clean: always
	rm -rf $(BUILD_DIR)/*

always:
	mkdir -p $(BUILD_DIR)/
