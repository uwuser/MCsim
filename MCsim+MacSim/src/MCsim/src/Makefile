#CXX = clang++
CXX = g++
#CXXFLAGS= -O3  -Wall 
CXXFLAGS= -O3 -std=c++0x -g -pg -Wall -Wpedantic 
# ######################################################
# 		Addjust the following option based on need
#CXXFLAGS += -DDEBUG_ENABLED
#CXXFLAGS += -DCMD_TRACE_ENABLED
#CXXFLAGS += -DREQ_TRACE_ENABLED
#CXXFLAGS += -DSINGLE_RANK_BANK_PRIVATIZATION 
#CXXFLAGS += -DMULTI_RANK_BANK_PRIVATIZATION 
########################################################
#-Wextra

EXE_NAME=MCsim
STATIC_LIB_NAME := libmcsim.a
LIB_NAME=libmcsim.so
LIB_NAME_MACOS=libmcsim.dylib

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
	g++ -std=c++11 -O3 -Wall -o $@ $^ 
	@echo "Built $@ successfully" 

$(LIB_NAME): $(POBJ)
	g++ -g -std=c++11 -shared -Wl,-soname,$@ -o $@ $^
	@echo "Built $@ successfully"

$(STATIC_LIB_NAME): $(LIB_OBJ)
	$(AR) crs $@ $^

$(LIB_NAME_MACOS): $(POBJ)
	g++ -std=c++11 -dynamiclib -o $@ $^
	@echo "Built $@ successfully"


# build all .cpp files to .o files
%.o : %.cpp
	$(CXX) $(CXXFLAGS) -o $@ -c $<
	
clean: 
	-rm -f $(REBUILDABLES) *.dep *.deppo *~ *.o *.so
