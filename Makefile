include ./Makefile.common
include ./vis_src/Makefile
include ./net_src/Makefile

$(NET_OBJ_OUT_PATH)server.o: ./server2016_src/server.cpp
	$(CXX) --std=c++14 -c -o $@ $(NET_INC_FLAGS) $(VIS_INC_FLAGS) $<

$(OUTDIR)/server2016: $(NET_OBJ_OUT_PATH)server.o $(OUTDIR)/lib5002-vis.so $(OUTDIR)/lib5002-net.so
	$(CXX) -o $(OUTDIR)/server2016 $< -L$(OUTDIR) -pthread -l5002-net -l5002-vis $(NET_LIB_FLAGS) $(VIS_LIB_FLAGS) -ldl

server2016: $(OUTDIR)/server2016

PROGRAMS += server2016




outdirs:
	$(foreach dir, $(OUTDIRS), mkdir -p $(dir) ;)

all: outdirs $(MODULES) $(PROGRAMS)

modules: outdirs $(MODULES)

programs: outdirs $(PROGRAMS)

clean:
	rm -rf $(OUTDIR)
	$(foreach dir, $(OUTDIRS), mkdir -p $(dir) ;)

.PHONY: $(MODULES) $(PROGRAMS) all modules programs outdirs clean
