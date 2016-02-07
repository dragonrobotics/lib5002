$(TRAJ_OBJ_FILES): $(TRAJ_OBJ_DIR)/%.o : $(TRAJ_SRC_DIR)/%.cpp
	@$(CXX) --std=c++14 -I$(TRAJ_INC_DIR) -c $< -o $@
