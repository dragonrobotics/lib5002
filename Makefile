ifndef ARCH
ARCH := X86
endif

ifeq ($(ARCH), ARM)
CXX := arm-linux-gnueabihf-g++
else
CXX := g++
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
OPENCV_LIB_FLAGS := -lopencv_core -lopencv_imgproc -lopencv_highgui -lopencv_imgcodecs
VISN_CPP_FILES := $(shell find $(VISN_SRC_DIR) -type f -name *.cpp)

CXXFLAGS := -std=c++14

Trajectory: trajectory

VisionProcessing: visproc

trajectory: $(TRAJ_OBJ_FILES)
	ar -rvs trajectory.a $(TRAJ_OBJ_FILES)

visproc: $(VISN_CPP_FILES)
	g++ -o visproc $(CXXFLAGS) -I$(VISN_INC_DIR) -I$(OPENCV_INC_DIR)  $(VISN_CPP_FILES) $(OPENCV_LIB_FLAGS)

$(TRAJ_OBJ_FILES): $(TRAJ_OBJ_DIR)/%.o : $(TRAJ_SRC_DIR)/%.cpp
	@$(CXX) $(CXXFLAGS) -I$(TRAJ_INC_DIR) -c $< -o $@
