include ../Makefile.common

COMMON_SOURCE_FILES := common.cpp  
COMMON_HEADER_FILES := visproc_common.h visproc_interface.h
COMMON_OBJECT_FILES := $(addsuffix .o, $(basename $(COMMON_SOURCE_FILES)))
INCLUDE_DIRS := ./include ./opencv_include

SRC_COM_PATH := $(addprefix ./, $(COMMON_SOURCE_FILES))
INC_COM_PATH := $(addprefix ./include/, $(COMMON_HEADER_FILES))
OBJ_COM_PATH := $(addprefix ./bin/, $(COMMON_OBJECT_FILES))

INC_FLAGS := $(addprefix -I, $(INCLUDE_DIRS))
LIB_FLAGS := -L/usr/local/lib -lopencv_shape -lopencv_stitching -lopencv_objdetect -lopencv_superres -lopencv_videostab -lopencv_calib3d -lopencv_features2d -lopencv_highgui -lopencv_videoio -lopencv_imgcodecs -lopencv_video -lopencv_photo -lopencv_ml -lopencv_imgproc -lopencv_flann -lopencv_core 

./bin/%.o : ./%.cpp $(INC_COM_PATH) 
	$(CXX) --std=c++14 -c -o $@ $(INC_FLAGS) $<

visproc-lib: $(OBJ_COM_PATH) ./bin/goal.o ./bin/boulder.o
	$(AR) rvs ./bin/libvisproc-5002.a $^

goalproc: $(OBJ_COM_PATH) ./bin/testing_environment.cpp ./bin/goal.o
	$(CXX) -o ./bin/goalproc $^ $(LIB_FLAGS)
	$(CP) ./bin/goalproc ../goalproc

ballproc: $(OBJ_COM_PATH) ./bin/testing_environment.cpp ./bin/boulder.o
	$(CXX) -o ./bin/ballproc $^ $(LIB_FLAGS)
	$(CP) ./bin/ballproc ../ballproc
