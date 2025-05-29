# Output binary
BIN = netstring

# Put all auto generated stuff to this build dir.
BUILD_DIR = ./build

SOURCE_DIR = src

# List of all the source files.
SOURCE = \
	main.cpp \
	mapwindow.cpp \
	sniffer.cpp \
	tui.cpp \

INCLUDES = \
	src \

CXX = g++

CXX_FLAGS = \
	-g -Wall -Wextra -Wfloat-equal -Wswitch-default -Wswitch-enum \
	${shell pkg-config --cflags chafa} \
	${shell pkg-config --cflags cairo} \
	${shell pkg-config --cflags libtins} \
	${shell pkg-config --cflags ncurses} \
	
LNK_FLAGS = \
	${shell pkg-config --libs chafa} \
	${shell pkg-config --libs cairo} \
	${shell pkg-config --libs libtins} \
	${shell pkg-config --libs ncurses} \

# All .o files go to build dir.
CXX_OBJ = $(SOURCE:%.cpp=$(BUILD_DIR)/%.o)
OBJ = $(CXX_OBJ)


# Gcc/Clang will create these .d files containing dependencies.
DEP = $(OBJ:%.o=%.d)
# Include paths with a -I in front of them
INCLUDEPATHS = $(INCLUDES:%= -I %)

# Actual target of the binary - depends on all .o files.
$(BIN) : PreBuild $(OBJ)
	echo Linking...
	# Create build directories - same structure as sources.
	-mkdir -p $(@D) 2>/dev/null
	$(CXX) $(OBJ) $(LNK_FLAGS) -o $(BIN)

# Include all .d files
-include $(DEP)

# Build target for every single object file.
# The potential dependency on header files is covered
# by calling `-include $(DEP)`.
$(BUILD_DIR)/%.o : $(SOURCE_DIR)/%.cpp
	echo Compiling $(notdir $<)
	-mkdir -p $(@D) 2>/dev/null
	# The -MMD flags additionaly creates a .d file with
	# the same name as the .o file.
	$(CXX) $(CXX_FLAGS) $(INCLUDEPATHS) -MMD -c $< -o $@


.PHONY : clean
clean:
	-rm -r $(BUILD_DIR)
	-rm $(BIN)

.PHONY: PreBuild
PreBuild:
