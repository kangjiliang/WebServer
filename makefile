

INCLUDE = -I./socket \
		  -I./main \
		  -I./public \
		  -I./codec \
		  -I./jsoncpp

CXX     = g++
FLAGS   = -Wall -g

##### webserver
WEBSOURCE  = $(wildcard ./main/*.cpp) $(wildcard ./socket/*.cpp) $(wildcard ./codec/*.cpp)
WEBTARGET  = webserver
WEBLIBS    = -lpthread

##### jsoncpp
JSONTARGET = ./cgibin/libjsoncpp.so
JSONSOURCE = $(wildcard ./jsoncpp/*.cpp)
JSONFLAGS  = $(FLAGS) -fPIC -shared

##### cgibin
CGITARGET  = cgitest
CGISRC     = ./cgisrc/cgibase.cpp ./cgisrc/$@.cpp
CGIBIN     = ./cgibin
CGILIBS    = -ljsoncpp
CGIFLAGS   = $(FLAGS) -Wl,-rpath=./cgibin
CGILIBDIR  = -L./cgibin

.PHONY: $(WEBTARGET) $(CGITARGET)

$(WEBTARGET):
	$(CXX) -o $@ $(WEBSOURCE) $(WEBLIBS) $(INCLUDE) $(FLAGS)

$(JSONTARGET):$(CGIBIN)
	$(CXX) -o $@ $(JSONSOURCE) $(JSONFLAGS) $(INCLUDE)

$(CGIBIN):
	@mkdir -pv $(CGIBIN)

$(CGITARGET):$(CGIBIN) $(JSONTARGET)
	$(CXX) -o $(CGIBIN)/$@ $(CGISRC) $(INCLUDE) $(CGILIBS) $(CGILIBDIR) $(CGIFLAGS)

cgiall:$(CGITARGET)
	@echo "done!"

cleanall:
	rm -rf $(WEBTARGET)
	rm -rf $(JSONTARGET)
	rm -rf $(CGIBIN)

clean:
	rm -rf $(WEBTARGET)

