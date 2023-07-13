CXX =g++
CFLAGS =-g3 -Wall -Wpedantic -std=c++20 -Lbuild
LIBS=-lpthread -lSFW
WORKDIR=$(shell pwd)
DEPENDENCIES=$(WORKDIR)/dependencies

INCLUDE=-I$(WORKDIR)/include/ \
		-I$(DEPENDENCIES)/SFW/include \
		-I$(DEPENDENCIES)/nlohmann-json/single_include \
		-I$(DEPENDENCIES)/spdlog/include \

INTERMEDIATES= $(WORKDIR)/intermediate/
OUTPUT = $(WORKDIR)/build/

export WORKDIR
export CFLAGS
export LIBS
export INCLUDE
export INTERMEDIATES
export CXX
export OUTPUT 

.PHONY: all deps link compile clean veryclean

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

veryclean: clean
	rm -f build/*
	$(MAKE) clean -C $(DEPENDENCIES)