

INCLUDE = -I./socket \
		  -I./main \
		  -I./public \
		  -I./codec

CXX     = g++
FLAGS   = -Wall -g

##### webserver
WEBSOURCE = $(wildcard ./main/*.cpp) $(wildcard ./socket/*.cpp) $(wildcard ./codec/*.cpp)
WEBTARGET = webserver
WEBLIBS   = -lpthread

.PHONY: $(WEBTARGET)

$(WEBTARGET):
	$(CXX) -o $(WEBTARGET) $(WEBSOURCE) $(WEBLIBS) $(INCLUDE) $(FLAGS)

clean:
	rm -rf $(WEBTARGET)

