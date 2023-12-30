SRC= \
	main.cpp \
	utils.cpp \
	MinecraftHandler.cpp \
	PlayerHandler.cpp \
	ServerPackets.cpp \
	DataTypes/nbt.cpp \
	DataTypes/Identifier.cpp \

OBJECT=$(SRC:.cpp=.o)

.PHONY: all 

all:$(OBJECT)

%.o:%.cpp
	$(CXX) -c -o $(INTERMEDIATES)$(notdir $@) $< $(INCLUDE) $(CFLAGS)

