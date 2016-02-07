VIS_COMMON_SOURCE_FILES := common.cpp  
VIS_COMMON_HEADER_FILES := visproc_common.h visproc_interface.h
VIS_COMMON_OBJECT_FILES := $(addsuffix .o, $(basename $(VIS_COMMON_SOURCE_FILES)))
VIS_INCLUDE_DIRS := ./visproc_src/include ./visproc_src/opencv_include

VIS_SRC_COM_PATH := $(addprefix ./visproc_src/, $(VIS_COMMON_SOURCE_FILES))
VIS_INC_COM_PATH := $(addprefix ./visproc_src/include/, $(VIS_COMMON_HEADER_FILES))
VIS_OBJ_COM_PATH := $(addprefix ./visproc_src/bin/, $(VIS_COMMON_OBJECT_FILES))

VIS_INC_FLAGS := $(addprefix -I, $(VIS_INCLUDE_DIRS))
VIS_LIB_FLAGS := -L/usr/local/lib -lopencv_shape -lopencv_stitching -lopencv_objdetect -lopencv_superres -lopencv_videostab -lopencv_calib3d -lopencv_features2d -lopencv_highgui -lopencv_videoio -lopencv_imgcodecs -lopencv_video -lopencv_photo -lopencv_ml -lopencv_imgproc -lopencv_flann -lopencv_core 

./visproc_src/bin/%.o : ./visproc_src/%.cpp $(VIS_INC_COM_PATH) 
	$(CXX) --std=c++14 -c -o $@ $(VIS_INC_FLAGS) $<

lib5002-vis.a: $(VIS_OBJ_COM_PATH) ./visproc_src/bin/goal.o ./visproc_src/bin/boulder.o
	$(AR) rvs lib5002-vis.a $^

goalproc: $(VIS_OBJ_COM_PATH) ./visproc_src/bin/testing_environment.o ./visproc_src/bin/goal.o
	$(CXX) -o goalproc $^ $(VIS_LIB_FLAGS)
	
ballproc: $(VIS_OBJ_COM_PATH) ./visproc_src/bin/testing_environment.o ./visproc_src/bin/boulder.o
	$(CXX) -o ballproc $^ $(VIS_LIB_FLAGS)
