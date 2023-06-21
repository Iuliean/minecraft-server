SRC= main.cpp\

OBJECT=$(addprefix $(INTERMEDIATES), $(SRC:.cpp=.o))

.PHONY: all

all: $(OBJECT)

$(INTERMEDIATES)%.o:%.cpp
	$(CXX) -c -o $@ $< $(INCLUDE) $(CFLAGS)