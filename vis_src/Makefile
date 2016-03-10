VIS_COMMON_SOURCE_FILES := common.cpp  
VIS_COMMON_HEADER_FILES := visproc_common.h visproc_interface.h
VIS_COMMON_OBJECT_FILES := $(addsuffix .o, $(basename $(VIS_COMMON_SOURCE_FILES)))
VIS_INCLUDE_DIRS := ./vis_src/include ./vis_src/opencv_include

VIS_OBJ_OUT_PATH := $(OUTDIR)/vis/

OUTDIRS += $(VIS_OBJ_OUT_PATH)

VIS_SRC_COM_PATH := $(addprefix ./vis_src/, $(VIS_COMMON_SOURCE_FILES))
VIS_INC_COM_PATH := $(addprefix ./vis_src/include/, $(VIS_COMMON_HEADER_FILES))
VIS_OBJ_COM_PATH := $(addprefix $(VIS_OBJ_OUT_PATH), $(VIS_COMMON_OBJECT_FILES))

VIS_INC_FLAGS := $(addprefix -I, $(VIS_INCLUDE_DIRS))
ifeq ($(ARCH), X86-64)
VIS_LIB_FLAGS := -L$(OPENCV-DIR) -lopencv_imgcodecs -lopencv_shape -lopencv_stitching -lopencv_objdetect -lopencv_superres -lopencv_videostab -lopencv_calib3d -lopencv_features2d -lopencv_highgui -lopencv_videoio -lopencv_video -lopencv_photo -lopencv_ml -lopencv_imgproc -lopencv_flann -lopencv_core $(OPENCV-3RDPARTY-DIR)/libzlib.a
endif

ifeq ($(ARCH), ARM)
VIS_LIB_FLAGS := -L$(OPENCV-DIR) -lopencv_imgcodecs -lopencv_shape -lopencv_videoio -lopencv_video -lopencv_imgproc -lopencv_core $(OPENCV-3RDPARTY-DIR)/libzlib.a
endif

$(VIS_OBJ_OUT_PATH):
	$(MKDIR) -p $@

$(VIS_OBJ_OUT_PATH)%.o : ./vis_src/%.cpp $(VIS_INC_COM_PATH) 
	$(CXX) --std=c++14 -fPIC -c -o $@ $(VIS_INC_FLAGS) $<

$(OUTDIR)/lib5002-vis.so: $(VIS_OBJ_COM_PATH) $(VIS_OBJ_OUT_PATH)goal.o $(VIS_OBJ_OUT_PATH)boulder.o
	$(CXX) --std=c++14 -fPIC -shared -o $(OUTDIR)/lib5002-vis.so $^



$(VIS_OBJ_OUT_PATH)testing_environment_goal.o : ./vis_src/testing_environment.cpp $(VIS_INC_COM_PATH)
	$(CXX) --std=c++14 -c -DVISPROC_EXTENDED_TESTING -DVISPROC_GOAL_TEST -o $@ $(VIS_INC_FLAGS) $< 

$(VIS_OBJ_OUT_PATH)testing_environment_ball.o : ./vis_src/testing_environment.cpp $(VIS_INC_COM_PATH)
	$(CXX) --std=c++14 -c -DVISPROC_EXTENDED_TESTING -DVISPROC_BALL_TEST -o $@ $(VIS_INC_FLAGS) $<

$(VIS_OBJ_OUT_PATH)testing_environment_basic.o : ./vis_src/testing_environment.cpp $(VIS_INC_COM_PATH)
	$(CXX) --std=c++14 -c -DVISPROC_BASIC_TESTING -o $@ $(VIS_INC_FLAGS) $<



$(OUTDIR)/goalproc: $(VIS_OBJ_OUT_PATH)testing_environment_goal.o $(OUTDIR)/lib5002-vis.so
	$(CXX) -o $(OUTDIR)/goalproc $^ $(VIS_LIB_FLAGS)

$(OUTDIR)/goalproc-basic: $(VIS_OBJ_OUT_PATH)testing_environment_basic.o $(OUTDIR)/lib5002-vis.so
	$(CXX) -o $(OUTDIR)/goalproc-basic $^ $(VIS_LIB_FLAGS)

$(OUTDIR)/ballproc: $(VIS_OBJ_OUT_PATH)testing_environment_ball.o $(OUTDIR)/lib5002-vis.so
	$(CXX) -o $(OUTDIR)/ballproc $^ $(VIS_LIB_FLAGS)

$(OUTDIR)/odometry: $(VIS_OBJ_OUT_PATH)visual_odometry.o $(OUTDIR)/lib5002-vis.so
	$(CXX) -o $(OUTDIR)/odometry $^ $(VIS_LIB_FLAGS)

lib5002-vis.so: $(OUTDIR)/lib5002-vis.so
goalproc: $(OUTDIR)/goalproc
goalproc-basic: $(OUTDIR)/goalproc-basic
ballproc: $(OUTDIR)/ballproc
odometry: $(OUTDIR)/odometry

MODULES += lib5002-vis.so
PROGRAMS += goalproc goalproc-basic ballproc
