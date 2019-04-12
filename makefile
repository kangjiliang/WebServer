SOURCES = $(wildcard ./socket/*.cpp) \
		  $(wildcard ./main/*.cpp)

INCLUDE = -I./socket \
		  -I./main

CXX     = g++
LIBS    = -lpthread
FLAGS   = -Wall -g
TARGET  = webserver


.PHONY: $(TARGET)

$(TARGET):
	$(CXX) -o $(TARGET) $(SOURCES) $(INCLUDE) $(FLAGS) $(LIBS)

clean:
	rm -rf $(TARGET)

