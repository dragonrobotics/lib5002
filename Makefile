ifndef ARCH
ARCH := X86
endif

ifeq ($(ARCH), ARM)
CXX := arm-linux-gnueabihf-g++
else
CXX := g++
endif

OBJ_DIR := ./obj
SRC_DIR := ./src
INC_DIR := ./src/include

CPP_FILES    := $(shell find $(SRC_DIR) -type f -name *.cpp)
OBJ_FILES :=  $(subst src/,obj/,$(patsubst %.cpp, %.o, $(CPP_FILES)))

CXXFLAGS := -I$(INC_DIR)

physsim.a: $(OBJ_FILES)
	ar rvs $(OBJ_FILES) physsim.a
	
$(OBJ_FILES): $(OBJ_DIR)/%.o : $(SRC_DIR)/%.cpp
	@$(CXX) $(CXXFLAGS) -c $< -o $@