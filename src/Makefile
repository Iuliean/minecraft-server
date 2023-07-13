SRC= \
	main.cpp \
	utils.cpp \
	MinecraftHandler.cpp \
	PlayerHandler.cpp

OBJECT=$(addprefix $(INTERMEDIATES), $(SRC:.cpp=.o))

.PHONY: all client 

all: client $(OBJECT)

$(INTERMEDIATES)%.o:%.cpp
	$(CXX) -c -o $@ $< $(INCLUDE) $(CFLAGS)

client:
	$(MAKE) -C client