include ./Makefile.common
include ./vis_src/Makefile
include ./net_src/Makefile

$(NET_OBJ_OUT_PATH)server.o: ./server2016_src/server.cpp
	$(CXX) --std=c++14 -c -o $@ $(NET_INC_FLAGS) $(VIS_INC_FLAGS) $<

$(NET_OBJ_OUT_PATH)discover_server.o: ./server2016_src/discserver.cpp
	$(CXX) --std=c++14 -c -o $@ $(NET_INC_FLAGS) $<

$(NET_OBJ_OUT_PATH)camera_server.o: ./server2016_src/visserver.cpp
	$(CXX) --std=c++14 -c -o $@ $(NET_INC_FLAGS) $(VIS_INC_FLAGS) $<

$(OUTDIR)/server2016: $(NET_OBJ_OUT_PATH)server.o $(OUTDIR)/lib5002-vis.so $(OUTDIR)/lib5002-net.so $(OUTDIR)/lib5002-stream.so
	$(CXX) -o $(OUTDIR)/server2016 $< -L$(OUTDIR) -l5002-stream -l5002-net -l5002-vis $(NET_LIB_FLAGS) $(VIS_LIB_FLAGS) -pthread -ldl

$(OUTDIR)/5002-discover: $(NET_OBJ_OUT_PATH)discover_server.o $(OUTDIR)/lib5002-net.so
	$(CXX) -o $(OUTDIR)/discover_server $< -L$(OUTDIR) -l5002-net $(NET_LIB_FLAGS) -pthread -ldl

$(OUTDIR)/5002-camera: $(NET_OBJ_OUT_PATH)camera_server.o $(OUTDIR)/lib5002-vis.so $(OUTDIR)/lib5002-net.so $(OUTDIR)/lib5002-stream.so
	$(CXX) -o $(OUTDIR)/camera_server $< -L$(OUTDIR) -l5002-stream -l5002-net -l5002-vis $(NET_LIB_FLAGS) $(VIS_LIB_FLAGS) -pthread -ldl


server2016: $(OUTDIR)/server2016
5002-discover: $(OUTDIR)/5002-discover
5002-camera: $(OUTDIR)/5002-camera

PROGRAMS += server2016 5002-discover 5002-camera

outdirs:
	$(foreach dir, $(OUTDIRS), mkdir -p $(dir) ;)

all: outdirs $(MODULES) $(PROGRAMS)

modules: outdirs $(MODULES)

programs: outdirs $(PROGRAMS)

clean:
	rm -rf $(OUTDIR)
	$(foreach dir, $(OUTDIRS), mkdir -p $(dir) ;)

.PHONY: $(MODULES) $(PROGRAMS) all modules programs outdirs clean
