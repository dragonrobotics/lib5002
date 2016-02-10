include ./Makefile.common
include ./visproc_src/Makefile
include ./server_src/Makefile

outdirs: $(OUTDIRS)
	mkdir -p $^
