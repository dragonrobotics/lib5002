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

VISN_COMMON := ./visproc_src/common.cpp ./visproc_src/testing_environment.cpp ./visproc_src/include/visproc_common.h ./visproc_src/include/visproc_interface.h

VISN_INC_DIR := $(VISN_SRC_DIR)/include
OPENCV_INC_DIR := $(VISN_SRC_DIR)/opencv_include
OPENCV_LIB_FLAGS := -lopencv_core -lopencv_imgproc -lopencv_highgui -lopencv_imgcodecs -lopencv_videoio

Trajectory: trajectory

VisionProcessing: visproc

trajectory: $(TRAJ_OBJ_FILES)
	ar -rvs trajectory.a $(TRAJ_OBJ_FILES)

goalproc: $(VISN_COMMON) ./visproc_src/goal.cpp
	$(X86CXX) --std=c++14 -o goalproc -I$(VISN_INC_DIR) -I$(OPENCV_INC_DIR) $(OPENCV_LIB_FLAGS) $(VISN_COMMON) ./visproc_src/goal.cpp

goalproc-arm: $(VISN_COMMON) ./visproc_src/goal.cpp
	$(ARMCXX) --std=c++14 -o goalproc -I$(VISN_INC_DIR) -I$(OPENCV_INC_DIR) $(OPENCV_LIB_FLAGS) $(VISN_COMMON) ./visproc_src/goal.cpp

ballproc: $(VISN_COMMON) ./visproc_src/goal.cpp
	$(X86CXX) --std=c++14 -o ballproc -I$(VISN_INC_DIR) -I$(OPENCV_INC_DIR) $(OPENCV_LIB_FLAGS) $(VISN_COMMON) ./visproc_src/ball.cpp

ballproc-arm: $(VISN_COMMON) ./visproc_src/goal.cpp
	$(ARMCXX) --std=c++14 -o ballproc -I$(VISN_INC_DIR) -I$(OPENCV_INC_DIR) $(OPENCV_LIB_FLAGS) $(VISN_COMMON) ./visproc_src/ball.cpp

nettest: ./server_src/test_server.cpp
	$(ARMCXX) --std=c++14 -o test_server_arm -I./server_src/include ./server_src/test_server.cpp

$(TRAJ_OBJ_FILES): $(TRAJ_OBJ_DIR)/%.o : $(TRAJ_SRC_DIR)/%.cpp
	@$(CXX) --std=c++14 -I$(TRAJ_INC_DIR) -c $< -o $@
