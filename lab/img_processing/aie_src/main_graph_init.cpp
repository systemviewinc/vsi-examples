// Memory Distributor
#include <stdio.h>
#include <stdlib.h>
#include <adf.h>

#include "example_processing_graph.cpp"

adf::simulation::platform<1, 1 > platform(
	"data/input.txt"
	,"data/output_0.txt"
);

kernel_graph bp_graph;

adf::connect<> input_net(platform.src[0], bp_graph.in);
adf::connect<> output_net_0(bp_graph.out_0, platform.sink[0]);

int main(int argc, char ** argv) { 
	printf("\n");
	printf("=============================\n");
	printf("=  BP 4 kernel processing   =\n");
	printf("=============================\n");

	bp_graph.init();
	bp_graph.run();
	bp_graph.end();

	printf("= Done =\n");
}

