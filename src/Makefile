#CXX = clang++
CXX = g++
CXXFLAGS= -O3 -std=c++0x -Wall -Wpedantic 
#CXXFLAGS= -O3 -std=c++0x -g -pg -Wall -Wpedantic 
# ######################################################
# 		Addjust the following option based on need
#CXXFLAGS += -DDEBUG_ENABLED
CXXFLAGS += -DCMD_TRACE_ENABLED
#CXXFLAGS += -DREQ_TRACE_ENABLED
CXXFLAGS += -DSINGLE_RANK_BANK_PRIVATIZATION 
#CXXFLAGS += -DMULTI_RANK_BANK_PRIVATIZATION 
########################################################
#-Wextra

EXE_NAME=MCsim

SRC = $(wildcard ../dram/*.cpp) $(wildcard *.cpp)
# $(info SRC is $(SRC))
OBJ = $(addsuffix .o, $(basename $(SRC)))

LIB_SRC := $(filter-out main.cpp,$(SRC))
# basename take the basic file name
LIB_OBJ := $(addsuffix .o, $(basename $(LIB_SRC)))

#build portable objects (i.e. with -fPIC)
# POBJ = $(addsuffix .po, $(basename $(LIB_SRC)))
# REBUILDABLES=$(OBJ) ${POBJ} $(EXE_NAME) $(LIB_NAME) $(STATIC_LIB_NAME)
REBUILDABLES=$(OBJ) $(EXE_NAME)

all: ${EXE_NAME}

#   $@ target name, $^ target deps, $< matched pattern
$(EXE_NAME): $(OBJ)
	$(CXX) $(CXXFLAGS) -o $@ $^ 
	@echo "Built $@ successfully" 

# build all .cpp files to .o files
%.o : %.cpp
	$(CXX) $(CXXFLAGS) -o $@ -c $<
	
clean: 
	-rm -f $(REBUILDABLES) *.dep *.deppo *~ *.o
