ifndef ARCH
ARCH := X86
endif

X86CXX := g++-4.9
ARMCXX := arm-linux-gnueabihf-g++-4.9

ifeq ($(ARCH), ARM)
CXX := $(ARMCXX)
else
CXX := $(X86CXX)
endif



TRAJ_SRC_DIR := ./trajectory_src
TRAJ_OBJ_DIR := $(TRAJ_SRC_DIR)/obj
TRAJ_INC_DIR := $(TRAJ_SRC_DIR)/include
TRAJ_CPP_FILES    := $(shell find $(TRAJ_SRC_DIR) -type f -name *.cpp)
TRAJ_OBJ_FILES :=  $(subst $(TRAJ_SRC_DIR),$(TRAJ_OBJ_DIR),$(patsubst %.cpp, %.o, $(TRAJ_CPP_FILES)))

VISN_SRC_DIR := ./visproc_src
VISN_OBJ_DIR := $(VISN_SRC_DIR)/obj
VISN_INC_DIR := $(VISN_SRC_DIR)/include
OPENCV_INC_DIR := $(VISN_SRC_DIR)/opencv_include
OPENCV_LIB_FLAGS := -lopencv_core -lopencv_imgproc -lopencv_highgui -lopencv_imgcodecs -lopencv_videoio
OPENCV_LIB_ARM_FLAGS := -Wl,-lopencv_core,-lopencv_imgproc,-lopencv_highgui,-lopencv_imgcodecs,-lopencv_videoio
VISN_CPP_FILES := ./visproc_src/visproc.cpp

Trajectory: trajectory

VisionProcessing: visproc

trajectory: $(TRAJ_OBJ_FILES)
	ar -rvs trajectory.a $(TRAJ_OBJ_FILES)

visproc-arm: $(VISN_CPP_FILES)
	$(ARMCXX) --std=c++14 -o visproc  -I$(VISN_INC_DIR) -I$(OPENCV_INC_DIR)  $(VISN_CPP_FILES) 

visproc: $(VISN_CPP_FILES)
	$(CXX) --std=c++14 -o visproc  -I$(VISN_INC_DIR) -I$(OPENCV_INC_DIR)  $(VISN_CPP_FILES) $(OPENCV_LIB_FLAGS)

nettest: ./server_src/test_server.cpp
	$(ARMCXX) --std=c++14 -o test_server_arm -I./server_src/include ./server_src/test_server.cpp

$(TRAJ_OBJ_FILES): $(TRAJ_OBJ_DIR)/%.o : $(TRAJ_SRC_DIR)/%.cpp
	@$(CXX) --std=c++14 -I$(TRAJ_INC_DIR) -c $< -o $@
