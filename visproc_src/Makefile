VIS_COMMON_SOURCE_FILES := common.cpp  
VIS_COMMON_HEADER_FILES := visproc_common.h visproc_interface.h
VIS_COMMON_OBJECT_FILES := $(addsuffix .o, $(basename $(VIS_COMMON_SOURCE_FILES)))
VIS_INCLUDE_DIRS := ./visproc_src/include ./visproc_src/opencv_include

VIS_OBJ_OUT_PATH := $(OUTDIR)/visproc/

OUTDIRS += $(VIS_OBJ_OUT_PATH)

VIS_SRC_COM_PATH := $(addprefix ./visproc_src/, $(VIS_COMMON_SOURCE_FILES))
VIS_INC_COM_PATH := $(addprefix ./visproc_src/include/, $(VIS_COMMON_HEADER_FILES))
VIS_OBJ_COM_PATH := $(addprefix $(VIS_OBJ_OUT_PATH), $(VIS_COMMON_OBJECT_FILES))

VIS_INC_FLAGS := $(addprefix -I, $(VIS_INCLUDE_DIRS))
VIS_LIB_FLAGS := -L/usr/local/lib -lopencv_shape -lopencv_stitching -lopencv_objdetect -lopencv_superres -lopencv_videostab -lopencv_calib3d -lopencv_features2d -lopencv_highgui -lopencv_videoio -lopencv_imgcodecs -lopencv_video -lopencv_photo -lopencv_ml -lopencv_imgproc -lopencv_flann -lopencv_core 

$(VIS_OBJ_OUT_PATH):
	$(MKDIR) -p $@

$(VIS_OBJ_OUT_PATH)%.o : ./visproc_src/%.cpp $(VIS_INC_COM_PATH) 
	$(CXX) --std=c++14 -c -o $@ $(VIS_INC_FLAGS) $<

$(OUTDIR)/lib5002-vis.a: $(VIS_OBJ_COM_PATH) $(VIS_OBJ_OUT_PATH)goal.o $(VIS_OBJ_OUT_PATH)boulder.o
	$(AR) rvs $(OUTDIR)/lib5002-vis.a $^

$(VIS_OBJ_OUT_PATH)testing_environment_goal.o : ./visproc_src/testing_environment.cpp $(VIS_INC_COM_PATH)
	$(CXX) --std=c++14 -c -DVISPROC_EXTENDED_TESTING -DVISPROC_GOAL_TEST -o $@ $(VIS_INC_FLAGS) $< 

$(VIS_OBJ_OUT_PATH)testing_environment_ball.o : ./visproc_src/testing_environment.cpp $(VIS_INC_COM_PATH)
	$(CXX) --std=c++14 -c -DVISPROC_EXTENDED_TESTING -DVISPROC_BALL_TEST -o $@ $(VIS_INC_FLAGS) $<

$(VIS_OBJ_OUT_PATH)testing_environment_basic.o : ./visproc_src/testing_environment.cpp $(VIS_INC_COM_PATH)
	$(CXX) --std=c++14 -c -DVISPROC_BASIC_TESTING -o $@ $(VIS_INC_FLAGS) $<

$(OUTDIR)/goalproc: $(VIS_OBJ_OUT_PATH)testing_environment_goal.o $(OUTDIR)/lib5002-vis.a
	$(CXX) -o $(OUTDIR)/goalproc $^ $(VIS_LIB_FLAGS)

$(OUTDIR)/goalproc-basic: $(VIS_OBJ_OUT_PATH)testing_environment_goal.o $(OUTDIR)/lib5002-vis.a
	$(CXX) -o $(OUTDIR)/goalproc $^ $(VIS_LIB_FLAGS)	

$(OUTDIR)/ballproc: $(VIS_OBJ_OUT_PATH)testing_environment_ball.o $(OUTDIR)/lib5002-vis.a
	$(CXX) -o $(OUTDIR)/ballproc $^ $(VIS_LIB_FLAGS)

lib5002-vis.a: $(OUTDIR)/lib5002-vis.a
goalproc: $(OUTDIR)/goalproc
goalproc-basic: $(OUTDIR)/goalproc-basic
ballproc: $(OUTDIR)/ballproc

MODULES += lib5002-vis.a
PROGRAMS += goalproc goalproc-basic ballproc
