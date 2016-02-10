NET_COMMON_SOURCE_FILES := netaddr.cpp netmsg.cpp sockwrap.cpp msgtype.cpp network_bytestream.cpp
NET_COMMON_HEADER_FILES := netaddr.h netmsg.h sockwrap.h msgtype.h network_bytestream.h
NET_COMMON_OBJECT_FILES := $(addsuffix .o, $(basename $(NET_COMMON_SOURCE_FILES)))
NET_INCLUDE_DIRS := ./server_src/include

NET_OBJ_OUT_PATH := $(OUTDIR)/net/

OUTDIRS += $(NET_OBJ_OUT_PATH)

NET_SRC_COM_PATH := $(addprefix ./server_src/, $(NET_COMMON_SOURCE_FILES))
NET_INC_COM_PATH := $(addprefix ./server_src/include/, $(NET_COMMON_HEADER_FILES))
NET_OBJ_COM_PATH := $(addprefix $(NET_OBJ_OUT_PATH), $(NET_COMMON_OBJECT_FILES))

NET_INC_FLAGS := $(addprefix -I, $(NET_INCLUDE_DIRS))
NET_LIB_FLAGS :=

$(NET_OBJ_OUT_PATH):
	$(MKDIR) -p $@

$(NET_OBJ_OUT_PATH)%.o : ./server_src/%.cpp $(NET_INC_COM_PATH)
	$(CXX) --std=c++14 -c -o $@ $(NET_INC_FLAGS) -Wno-pointer-arith $<

$(OUTDIR)/lib5002-net.a: $(NET_OBJ_COM_PATH)
	$(AR) rvs $(OUTDIR)/lib5002-net.a $^	

$(OUTDIR)/nettest: ./server_src/bin/test_server.o $(OUTDIR)/lib5002-net.a
	$(CXX) -o $(OUTDIR)/nettest $^ $(NET_LIB_FLAGS)

nettest: $(OUTDIR)/nettest
lib5002-net.a: $(OUTDIR)/lib5002-net.a
