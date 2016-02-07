include ../Makefile.common

COMMON_SOURCE_FILES := common.cpp testing_environment.cpp 
COMMON_HEADER_FILES := visproc_common.h visproc_interface.h
COMMON_OBJECT_FILES := $(addsuffix .o, $(basename $(COMMON_SOURCE_FILES)))
INCLUDE_DIRS := ./include ./opencv_include

SRC_COM_PATH := $(addprefix ./, $(COMMON_SOURCE_FILES))
INC_COM_PATH := $(addprefix ./include/, $(COMMON_HEADER_FILES))
OBJ_COM_PATH := $(addprefix ./bin/, $(COMMON_SOURCE_FILES))

INC_FLAGS := $(addprefix -I, $(INCLUDE_DIRS))
LIB_FLAGS := -L/usr/local/lib -lopencv_shape -lopencv_stitching -lopencv_objdetect -lopencv_superres -lopencv_videostab -lopencv_calib3d -lopencv_features2d -lopencv_highgui -lopencv_videoio -lopencv_imgcodecs -lopencv_video -lopencv_photo -lopencv_ml -lopencv_imgproc -lopencv_flann -lopencv_core 

./bin/%.o : ./%.cpp $(INC_COM_PATH) 
	$(CXX) --std=c++14 -o ./bin/%.o $(INC_FLAGS) ./%.cpp |& tee --append build.out

visproc-common: $(OBJ_COM_PATH) $(INC_COM_PATH)

goalproc: visproc-common ./bin/goal.o
	$(RM) ./build.out
	$(LD) -o ./bin/goalproc $^ $(LIB_FLAGS)
	$(CP) ./bin/goalproc ../goalproc

ballproc: visproc-common ./bin/ball.o
	$(RM) ./build.out
	$(LD) -o ./bin/ballproc $^ $(LIB_FLAGS)
	$(CP) ./bin/ballproc ../ballproc
