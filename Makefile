include ./Makefile.common
include ./visproc_src/Makefile
include ./server_src/Makefile

outdirs:
	$(foreach dir, $(OUTDIRS), mkdir -p $(dir) ;)

all: outdirs $(MODULES) $(PROGRAMS)

modules: outdirs $(MODULES)

programs: outdirs $(PROGRAMS)

clean:
	rm -rf ./bin
	$(foreach dir, $(OUTDIRS), mkdir -p $(dir) ;)

.PHONY: $(MODULES) $(PROGRAMS) all modules programs outdirs clean
