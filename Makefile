NAME  = libafxelf
COLOR = 35

BUILD    := ./build/
BIN      := $(BUILD)$(NAME)/
BOOT     := boot/
DEP_DEST := $(BIN)dep/
OBJ_DEST := $(BIN)obj/

MKDIR = mkdir -p

CXXFILES := $(shell find . -type f -name '*.cpp')
HXXFILES := $(shell find . -type f -name '*.hpp')

OBJS := $(patsubst %.o, $(OBJ_DEST)%.o, $(CXXFILES:.cpp=.cpp.o))

GFLAGS   := $(GFLAGS) -O3 -Wall -Wextra -nostdlib -pipe -lgcc -ffreestanding -g
INCLUDES := -I. -Iinclude

CXXFLAGS := $(GFLAGS) $(CXXFLAGS)   \
	-std=c++20				   		\
	-fno-rtti				   		\
	-fno-exceptions			   		\
	-fno-pic			   	   		\
	-fno-stack-protector       		\
	-fno-omit-frame-pointer    		\
	-fvisibility=hidden		   		\
	$(INCLUDES)

all: $(OBJS)
	@$(AR) rcs -o $(BIN)$(NAME).a $(OBJS)
	@printf '\033[0;$(COLOR)m%s\033[0m: Done building \033[0;$(COLOR)m%s\033[0m\033[0K\n' $(NAME)

include $(shell find $(DEP_DEST) -type f -name *.d)

clean:
	rm -rf $(DEP_DEST)
	rm -rf $(OBJ_DEST)

$(OBJ_DEST)%.cpp.o : %.cpp
	@$(MKDIR) -p ${@D}
	@$(MKDIR) -p $(dir $(DEP_DEST)$*)

	@printf '\033[0;$(COLOR)m%s\033[0m: Building \033[0;$(COLOR)m$(<)\033[0m\033[0K\n' $(NAME)
	@$(CXX) $(CXXFLAGS) -c $< -o $@ -MMD -MT $@ -MF $(DEP_DEST)$*.cpp.d
