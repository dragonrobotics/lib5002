include ../Makefile.common

COMMON_SOURCE_FILES := netaddr.cpp netmsg.cpp sockwrap.cpp msgtype.cpp
COMMON_HEADER_FILES := netaddr.h netmsg.h sockwrap.h msgtype.h
COMMON_OBJECT_FILES := $(addsuffix .o, $(basename $(COMMON_SOURCE_FILES)))
INCLUDE_DIRS := ./include ../visproc_src/include

SRC_COM_PATH := $(addprefix ./, $(COMMON_SOURCE_FILES))
INC_COM_PATH := $(addprefix ./include/, $(COMMON_HEADER_FILES))
OBJ_COM_PATH := $(addprefix ./bin/, $(COMMON_OBJECT_FILES))

INC_FLAGS := $(addprefix -I, $(INCLUDE_DIRS))
LIB_FLAGS := -L/usr/local/lib -lopencv_shape -lopencv_stitching -lopencv_objdetect -lopencv_superres -lopencv_videostab -lopencv_calib3d -lopencv_features2d -lopencv_highgui -lopencv_videoio -lopencv_imgcodecs -lopencv_video -lopencv_photo -lopencv_ml -lopencv_imgproc -lopencv_flann -lopencv_core 

./bin/%.o : ./%.cpp $(INC_COM_PATH)
	$(CXX) --std=c++14 -c -o $@ $(INC_FLAGS) -Wno-pointer-arith $<

nettest: $(OBJ_COM_PATH) ./bin/server.o
	$(CXX) -o ./bin/nettest $^ $(LIB_FLAGS)
	$(CP) ./bin/nettest ../nettest
