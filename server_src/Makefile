NET_COMMON_SOURCE_FILES := netaddr.cpp netmsg.cpp sockwrap.cpp msgtype.cpp
NET_COMMON_HEADER_FILES := netaddr.h netmsg.h sockwrap.h msgtype.h
NET_COMMON_OBJECT_FILES := $(addsuffix .o, $(basename $(NET_COMMON_SOURCE_FILES)))
NET_INCLUDE_DIRS := ./server_src/include ./visproc_src/include

NET_SRC_COM_PATH := $(addprefix ./server_src/, $(NET_COMMON_SOURCE_FILES))
NET_INC_COM_PATH := $(addprefix ./server_src/include/, $(NET_COMMON_HEADER_FILES))
NET_OBJ_COM_PATH := $(addprefix ./server_src/bin/, $(NET_COMMON_OBJECT_FILES))

NET_INC_FLAGS := $(addprefix -I, $(NET_INCLUDE_DIRS))
NET_LIB_FLAGS := -L/usr/local/lib -lopencv_shape -lopencv_stitching -lopencv_objdetect -lopencv_superres -lopencv_videostab -lopencv_calib3d -lopencv_features2d -lopencv_highgui -lopencv_videoio -lopencv_imgcodecs -lopencv_video -lopencv_photo -lopencv_ml -lopencv_imgproc -lopencv_flann -lopencv_core

./server_src/bin/%.o : ./server_src/%.cpp $(NET_INC_COM_PATH)
	$(CXX) --std=c++14 -c -o $@ $(NET_INC_FLAGS) -Wno-pointer-arith $<

lib5002-net.a: $(NET_OBJ_COM_PATH)
	$(AR) rvs lib5002-net.a $^
	
nettest: ./server_src/bin/server.o lib5002-net.a lib5002-vis.a
	$(CXX) -o nettest $^ $(NET_LIB_FLAGS)
