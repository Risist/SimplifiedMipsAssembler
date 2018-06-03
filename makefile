CC = g++
FLAGS = -std=c++14 -Wall -Wextra -Werror 

TARGET = mipsAssembler

SRC = $(wildcard *.cpp)
OBJ = $(patsubst %.cpp, %.o, $(SRC))

$(TARGET) : $(OBJ)
	$(CC) $(FLAGS) -o $(TARGET) $(OBJ)
%.o : %.cpp
	$(CC) $(FLAGS) -c $< -o $@
clean:
	$(RM) $(TARGET)

.PHONY: depend clean
	
