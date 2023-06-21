CXX =g++
CFLAGS =-g3 -Wall -Wpedantic -std=c++20
LIBS=-lpthread
WORKDIR=$(shell pwd)
DEPENDENCIES=$(WORKDIR)/dependencies

INCLUDE=-I$(WORKDIR)/include/ \
		-I$(DEPENDENCIES)/SFW/include \
		-I$(DEPENDENCIES)/spdlog/include \

INTERMEDIATES= $(WORKDIR)/intermediate/
OUTPUT = $(WORKDIR)/build/

export CFLAGS
export LIBS
export INCLUDE
export INTERMEDIATES
export CXX
export OUTPUT 

.PHONY: all deps link compile clean

all: deps compile link

deps:
	$(MAKE) -C $(DEPENDENCIES)

compile: deps
	mkdir -p $(OUTPUT)
	mkdir -p $(INTERMEDIATES)
	$(MAKE) -C src/


link:compile
	$(CXX) -o build/server $(shell ls $(INTERMEDIATES)*.o) $(CFLAGS) $(LIBS)

clean:
	rm -f intermediate/*