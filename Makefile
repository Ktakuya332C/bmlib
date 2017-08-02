CXX = g++
CXXFLAGS = -Wall -Ibmlib -std=c++11 -g

.PHONY: example
example: build/train_rbm

build/train_rbm: build/train_rbm.o build/graph.o build/bm.o build/math_utils.o
	$(CXX) $(CXXFLAGS) $^ -o $@

build/train_rbm.o: example/train_rbm.cc
	$(CXX) $(CXXFLAGS) -c $^ -o $@

.PHONY: test
test: build/graph_test build/bm_test

build/bm_test: build/bm_test.o build/bm.o build/graph.o build/math_utils.o
	$(CXX) $(CXXFLAGS) $^ -o $@

build/bm.o: bmlib/bm.cc
	$(CXX) $(CXXFLAGS) -c $^ -o $@

build/bm_test.o: bmlib/bm_test.cc
	$(CXX) $(CXXFLAGS) -c $^ -o $@

build/graph_test: build/graph_test.o build/graph.o
	$(CXX) $(CXXFLAGS) $^ -o $@

build/graph.o: bmlib/graph.cc
	$(CXX) $(CXXFLAGS) -c $^ -o $@

build/graph_test.o: bmlib/graph_test.cc
	$(CXX) $(CXXFLAGS) -c $^ -o $@


build/math_utils.o: bmlib/math_utils.cc
	$(CXX) $(CXXFLAGS) -c $^ -o $@

.PHONY: clean
clean:
	rm -r build/*
