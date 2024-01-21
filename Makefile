NAME  := libafxelf
TYPE  := lib
OPTIM := 0
FREE  := true # freestanding
NOSTD := true # nostdlib
BARE  := true # no exceptions, rtti, etc
PIE   ?= false # pie, pic
DEBUG ?= true # debug
TRACE ?= true # allow stack tracing
LIBS  := 

CXXFLAGS := -std=c++20
INCLUDES := -I. -I./include

ifeq ($(TYPE), lib)
	COLOR := 35
else
	COLOR := 32
endif

ROOT     ?= $(shell dirname $(realpath $(firstword $(MAKEFILE_LIST))))
BUILD    := $(ROOT)/build/
BIN      := $(BUILD)$(NAME)/
DEP_DEST := $(BIN)dep/
OBJ_DEST := $(BIN)obj/

export ROOT
export DEBUG
export TRACE

INCLUDES := $(INCLUDES) $(foreach lib,$(LIBS),-I./lib/$(lib)/include )
EXCLUDES := $(foreach lib,$(LIBS),./lib/$(lib)/% )

CFILES   := $(filter-out $(EXCLUDES),$(shell find . -type f -name '*.c'))
CXXFILES := $(filter-out $(EXCLUDES),$(shell find . -type f -name '*.cpp'))

OBJS  := $(patsubst %.o, $(OBJ_DEST)%.o, $(CXXFILES:.cpp=.cpp.o))
LINKS := $(foreach lib,$(LIBS),./build/$(lib)/$(lib).a )

CXXFLAGS += -O$(OPTIM) -Wall -Wextra -pipe -lgcc $(INCLUDES)

ifeq ($(strip $(FREE)), true)
	CXXFLAGS += -ffreestanding
endif

ifeq ($(strip $(NOSTD)), true)
	CXXFLAGS += -nostdlib
endif

ifeq ($(strip $(DEBUG)), true)
	CXXFLAGS += -g
endif

ifeq ($(strip $(BARE)), true)
	CXXFLAGS += -fno-rtti
	CXXFLAGS += -fno-exceptions
endif

ifneq ($(strip $(PIE)), true)
	CXXFLAGS += -fno-pic
	CXXFLAGS += -no-pie
endif

ifeq ($(strip $(TRACE)), true)
	CXXFLAGS += -fno-stack-protector 
	CXXFLAGS += -fno-omit-frame-pointer
endif

define submake
	$(MAKE) -C $(1) $(2)

endef

all: $(OBJS)
	$(foreach lib,$(LIBS),$(call submake, ./lib/$(lib), all))

ifeq ($(TYPE), lib)
	@$(AR) rcs -o $(BIN)$(NAME).a $(OBJS)
else
	@$(CXX) $(CXXFLAGS) -o $(BIN)$(NAME) $(OBJS) $(LINKS)
endif
	@printf '\033[0;$(COLOR)m%s\033[0m: Done building \033[0;$(COLOR)m%s\033[0m\033[0K\n' $(NAME)

include $(shell find $(DEP_DEST) -type f -name *.d)

clean:
	$(foreach lib,$(LIBS),$(call submake, ./lib/$(lib), clean))

	@rm -rf $(DEP_DEST)
	@rm -rf $(OBJ_DEST)

$(OBJ_DEST)%.cpp.o : %.cpp
	mkdir -p ${@D}
	mkdir -p $(dir $(DEP_DEST)$*)

	@printf '\033[0;$(COLOR)m%s\033[0m: Building \033[0;$(COLOR)m$(<)\033[0m\033[0K\n' $(NAME)
	@$(CXX) $(CXXFLAGS) -c $< -o $@ -MMD -MT $@ -MF $(DEP_DEST)$*.cpp.d
 