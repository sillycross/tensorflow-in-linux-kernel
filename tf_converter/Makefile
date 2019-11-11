main: main.o cpu_function_runtime.o xla_compiled_cpu_function.o
	g++ main.o cpu_function_runtime.o xla_compiled_cpu_function.o input/libgraph.pic.a -o main

cpu_function_runtime.o: tensorflow_stub/tensorflow/compiler/xla/cpu_function_runtime.cc
	g++ -c tensorflow_stub/tensorflow/compiler/xla/cpu_function_runtime.cc -Itensorflow_stub
	
xla_compiled_cpu_function.o: tensorflow_stub/tensorflow/compiler/tf2xla/xla_compiled_cpu_function.cc
	g++ -c tensorflow_stub/tensorflow/compiler/tf2xla/xla_compiled_cpu_function.cc -Itensorflow_stub

main.o: main.cpp input/graph.h
	g++ -c main.cpp -Itensorflow_stub 

