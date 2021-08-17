#include "apps/eclipseworkspace/facial/RecogDecoder.hpp"

#include <iostream>
#include <math.h>
#include <chrono>
#include <ctime>

namespace isaac {

void RecogDecoder::start() {
	past_embeddings = Tensor2f(1,512);
	no_past = 1;
	tickOnMessage(rx_input());
}

void RecogDecoder::tick() {
	TensorConstView4f const_input;

	FromProto(rx_input().getProto(), rx_input().buffers(), const_input);

	//if there is exactly 1 face on the screen
	if(!(const_input.dimensions()[0]==1)) return;

	Tensor4f input(1,3,112,112);
	Copy(const_input,input);
	//to check output of detection extractor
	/*Tensor3f input(3,112,112);
	for(int ch = 0;ch<3;ch++){
		for(int r = 0;r<112;r++){
			for(int c = 0;c<112;c++){
				input(ch,r,c) = const_input(0,ch,r,c);
			}
		}
	}*/

	ToProto(std::move(input),tx_output().initProto(),tx_output().buffers());
	tx_output().publish();

	auto start = std::chrono::system_clock::now();
	while(!rx_embeddings().available()){
		auto end = std::chrono::system_clock::now();
		std::chrono::duration<double> elapsed_seconds = end-start;
		if(elapsed_seconds.count() > 2.0){
			return;
		}
	}
	TensorConstView2f const_embeddings;
	FromProto(rx_embeddings().getProto(), rx_embeddings().buffers(), const_embeddings);

	double sum=0;
	for(int i = 0;i<512;i++){
		sum+=pow((const_embeddings(0,i)-past_embeddings(0,i)),2.0);
	}
	sum = sqrt(sum);

	if(no_past){
		no_past = 0;
		Copy(const_embeddings,past_embeddings);
		return;
	}
	show("embedding distance",sum);
	if(abs(sum)>1.0){
		std::cout << "updated past with distance: " << sum <<  '\n';
		Copy(const_embeddings,past_embeddings);
	}
}
}
