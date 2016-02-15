NET_COMMON_SOURCE_FILES := netaddr.cpp netmsg.cpp sockwrap.cpp msgtype.cpp network_bytestream.cpp
NET_COMMON_HEADER_FILES := netaddr.h netmsg.h sockwrap.h msgtype.h network_bytestream.h
NET_COMMON_OBJECT_FILES := $(addsuffix .o, $(basename $(NET_COMMON_SOURCE_FILES)))
NET_INCLUDE_DIRS := ./net_src/include

NET_OBJ_OUT_PATH := $(OUTDIR)/net/

OUTDIRS += $(NET_OBJ_OUT_PATH)

NET_SRC_COM_PATH := $(addprefix ./net_src/, $(NET_COMMON_SOURCE_FILES))
NET_INC_COM_PATH := $(addprefix ./net_src/include/, $(NET_COMMON_HEADER_FILES))
NET_OBJ_COM_PATH := $(addprefix $(NET_OBJ_OUT_PATH), $(NET_COMMON_OBJECT_FILES))

NET_INC_FLAGS := $(addprefix -I, $(NET_INCLUDE_DIRS))
NET_LIB_FLAGS :=

$(NET_OBJ_OUT_PATH):
	$(MKDIR) -p $@

$(NET_OBJ_OUT_PATH)%.o : ./net_src/%.cpp $(NET_INC_COM_PATH)
	$(CXX) --std=c++14 -fPIC -c -o $@ $(NET_INC_FLAGS) -Wno-pointer-arith $<

$(NET_OBJ_OUT_PATH)video_stream.o : ./net_src/video_stream.cpp ./net_src/include/video_stream.h $(NET_INC_COM_PATH)
	$(CXX) --std=c++14 -fPIC -c -o $@ $(NET_INC_FLAGS) $(VIS_INC_FLAGS) -Wno-pointer-arith $<

$(NET_OBJ_OUT_PATH)wpilib_cameraserver.o : ./net_src/wpilib_cameraserver.cpp ./net_src/include/wpilib_cameraserver.h $(NET_INC_COM_PATH)
	$(CXX) --std=c++14 -fPIC -c -o $@ $(NET_INC_FLAGS) $(VIS_INC_FLAGS) -Wno-pointer-arith $<

$(OUTDIR)/lib5002-stream.so: $(NET_OBJ_OUT_PATH)video_stream.o $(NET_OBJ_OUT_PATH)wpilib_cameraserver.o
	$(CXX) --std=c++14 -fPIC -shared -o $(OUTDIR)/lib5002-stream.so $^	

$(OUTDIR)/lib5002-net.so: $(NET_OBJ_COM_PATH)
	$(CXX) --std=c++14 -fPIC -shared -o $(OUTDIR)/lib5002-net.so $^	

$(OUTDIR)/disctest: $(NET_OBJ_OUT_PATH)discovertest.o $(OUTDIR)/lib5002-net.so
	$(CXX) -o $(OUTDIR)/disctest $^ $(NET_LIB_FLAGS)

$(OUTDIR)/nettest: $(NET_OBJ_OUT_PATH)test_server.o $(OUTDIR)/lib5002-net.so
	$(CXX) -o $(OUTDIR)/nettest $^ $(NET_LIB_FLAGS)

disctest: $(OUTDIR)/disctest
nettest: $(OUTDIR)/nettest
lib5002-net.so: $(OUTDIR)/lib5002-net.so
lib5002-stream.so: $(OUTDIR)/lib5002-stream.so

MODULES += lib5002-net.so lib5002-stream.so
PROGRAMS += nettest disctest
