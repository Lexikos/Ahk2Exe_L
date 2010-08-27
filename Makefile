
#----------------
# The C and C++ compilers to use.
#----------------
CC = gcc
CCC = g++
CXX = $(CCC)


#----------------
#set directories for project
#----------------
INC_DIR = src
DOC_DIR = src
SRC_DIR = src
LIB_DIR = src/lib/mingw32
RES_DIR = src/resources


#----------------
# set compiler-options
#----------------
ifeq ($(DEBUG), YES)
	CFLAGS = -I $(INC_DIR) -D_DEBUG_ -g
	OBJ_DIR = debug
	EXE_DIR = bin
	LIB_ADD = _d
else
	#CFLAGS =-O -Wall -Wno-deprecated -mthreads -I $(INC_DIR)
	CFLAGS =-O1 -Wno-deprecated -mthreads -I $(INC_DIR)
	OBJ_DIR = release
	EXE_DIR = bin
	LIB_ADD = 
endif


#----------------
#set file groups
#----------------

OBJECTS =		$(OBJ_DIR)/application.o	\
			$(OBJ_DIR)/astring_datatype.o	\
			$(OBJ_DIR)/Aut2Exe.o		\
			$(OBJ_DIR)/cmdline.o		\
			$(OBJ_DIR)/util.o		\
			$(OBJ_DIR)/lexer.o		\
			$(OBJ_DIR)/variant_datatype.o	\
			$(OBJ_DIR)/token_datatype.o	\
			$(OBJ_DIR)/vector_token_datatype.o	\
			$(OBJ_DIR)/changeicon.o	\
			$(OBJ_DIR)/ResourceEditor.o	\
			$(OBJ_DIR)/Aut2Exe.res.o

LIBS	=	-lcomctl32 -lexearc_write
			

RESOURCES = $(RES_DIR)/Aut2Exe.rc

TARGET =	Aut2Exe


#----------------
#LinkerFlags
#----------------
LDFLAGS =  -mwindows -L$(LIB_DIR) 


#----------------
#*** Makerules ***
#----------------
.SUFFIXES: .o .cpp .c .rc
# allgemein Regeln

$(OBJ_DIR)/%.o : $(SRC_DIR)/%.c
	$(CC) $(CFLAGS) -c $< -o $(OBJ_DIR)/$*.o

$(OBJ_DIR)/%.o : $(SRC_DIR)/%.cpp
	$(CCC) $(CFLAGS) -c $< -o $(OBJ_DIR)/$*.o

$(OBJ_DIR)/%.res.o : $(RES_DIR)/%.rc
	windres --include-dir $(RES_DIR) -i $< -o $@


#----------------
#*** Targets  ***
#----------------

all: Aut2Exe

Aut2Exe: $(EXE_DIR)/$(TARGET).exe

$(EXE_DIR)/$(TARGET).exe : $(OBJECTS)
	$(CXX) $(LDFLAGS) $(CFLAGS) $(OBJECTS) -o $@ $(LIBS) -m486
	strip $@
	$(EXE_DIR)/upx.exe --best --compress-icons=0 $(EXE_DIR)/$(TARGET).exe


clean:
	rm -f $(OBJ_DIR)/*.o
	rm -f $(EXE_DIR)/$(TARGET).exe

