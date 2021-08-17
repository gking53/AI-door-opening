#include "apps/eclipseworkspace/facial/CudaDetection.hpp"

#include <string>
#include <vector>

#include "engine/core/tensor/tensor.hpp"
#include "engine/core/math/types.hpp"
#include "engine/gems/geometry/n_cuboid.hpp"

#include <iostream>

namespace isaac {

void CudaDetection::start() {
	tickOnMessage(rx_boxes());
}

void CudaDetection::tick() {

	  TensorConstView3f scores;
	  TensorConstView3f boxes;
	  FromProto(rx_scores().getProto(), rx_scores().buffers(), scores);
	  FromProto(rx_boxes().getProto(), rx_boxes().buffers(), boxes);
	  std::vector<std::pair<float,int>> numFaces= filterBoxes(scores,boxes);
	  //std::printf("\n\n");
	  //std::printf("next frame\n");

	  show("Number of faces",numFaces.size());

	  auto detections_proto = tx_detections().initProto();

	  auto boxes_proto = detections_proto.initBoundingBoxes(numFaces.size());
	  auto predictions_proto = detections_proto.initPredictions(numFaces.size());

	  for (size_t i = 0; i < numFaces.size(); i++) {
		  // Populate label and confidence
		  predictions_proto[i].setLabel("Face");
		  predictions_proto[i].setConfidence(numFaces[i].first);
		  // Populate bounding box rectangle information
		  //stupid sanity check cuz model goes outside of input image dimensions which causes every other codelet to try to access that memory outside of the image and fault
		  double c0,c1,c2,c3;
		  c0 = boxes(0,numFaces[i].second,0);
		  if(c0<0)c0=0.0;
		  else if(c0>1)c0=1.0;
		  c1 = boxes(0,numFaces[i].second,1);
		  if(c1<0)c1=0.0;
		  else if(c1>1)c1=1.0;
		  c2 = boxes(0,numFaces[i].second,2);
		  if(c2<0)c2=0.0;
		  else if(c2>1)c2=1.0;
		  c3 = boxes(0,numFaces[i].second,3);
		  if(c3<0)c3=0.0;
		  else if(c3>1)c3=1.0;
		  geometry::RectangleD bounding_box = geometry::RectangleD::FromOppositeCorners(
			        Vector2d(c1*get_rows(), c0*get_cols()),
			        Vector2d(c3*get_rows(), c2*get_cols()));
		  ToProto(bounding_box, boxes_proto[i]);
	  }
	  tx_detections().publish();
}

std::vector<std::pair<float,int>> CudaDetection::filterBoxes(TensorConstView3f& scores, TensorConstView3f& boxes){
	std::vector<std::pair<float,int>> picked;
	std::vector<std::pair<float,int>> scores_offsets;
	double box_threshold = get_box_threshold();
	double score_threshold = get_score_threshold();
	//get list of all potential boxes with high enough confidence
	for(int numOutputs = 0; numOutputs < scores.dimensions()[1]; numOutputs++){
		  if(scores(0,numOutputs,1)>score_threshold) scores_offsets.push_back(std::make_pair(scores(0,numOutputs,1),numOutputs));
	}
	//sort this vector pair in terms ascending score
	if(scores_offsets.empty()) return picked;
	std::sort(scores_offsets.begin(),scores_offsets.end());
	//go through from back to front and remove any boxes that overlap with a higher probability box
	while(!scores_offsets.empty()){
		picked.push_back(scores_offsets.back());
		//edge case for if 1 box is left
		if(scores_offsets.size()==1) break;
		std::vector<double> reference;
		double reference_area = 0;
		double x;
		double y;
		for(unsigned char box_iterator = 0;box_iterator<4;box_iterator++){
			reference.push_back(boxes(0,scores_offsets.back().second,box_iterator));
		}
		//area of box
		x = (reference[2]-reference[0]);
		if(x<0)x = 0;
		y = (reference[3]-reference[1]);
		if(y<0)y = 0;
		reference_area = x*y;
		std::vector<int> toBeRemoved;
		//std::printf("\nnext reference item\n");
		//std::printf("scores_offsets size: %d\n",scores_offsets.size());
		//std::printf("midpoint of reference %f %f\n",get_cols()*(reference[2]+reference[0])/2,get_rows()*(reference[3]+reference[1])/2);
		for(unsigned int scores_offsets_iterator = 0;scores_offsets_iterator < scores_offsets.size()-1;scores_offsets_iterator++){
			//compute jaccard index for reference box to current box
			std::vector<double> current;
			std::vector<double> intersection;
			double current_area;
			double intersection_area;
			for(unsigned char box_iterator = 0;box_iterator<4;box_iterator++){
				current.push_back(boxes(0,scores_offsets[scores_offsets_iterator].second,box_iterator));
			}
			x = (current[2]-current[0]);
			if(x<0)x = 0;
			y = (current[3]-current[1]);
			if(y<0)y = 0;
			current_area = x*y;
			intersection.push_back((current[0]<reference[0])?reference[0]:current[0]); //min
			intersection.push_back((current[1]<reference[1])?reference[1]:current[1]);
			intersection.push_back((current[2]>reference[2])?reference[2]:current[2]); //max
			intersection.push_back((current[3]>reference[3])?reference[3]:current[3]);
			x = (intersection[2]-intersection[0]);
			if(x<0)x = 0;
			y = (intersection[3]-intersection[1]);
			if(y<0)y = 0;
			intersection_area = x*y;
			double denom = reference_area + current_area - intersection_area;
			if(denom == 0) denom += 0.00001; //prolly small enough, theres definitly a better way of doing this
			double invDenom = 1/denom;
			double jaccard_index = intersection_area * invDenom;
			//queue removal of overlapping boxes
			//std::printf("midpoint of current %f %f\n",get_cols()*(current[2]+current[0])/2,get_rows()*(current[3]+current[1])/2);
			//std::printf("index %f to item %d\n",jaccard_index,picked.size());
			if(jaccard_index>box_threshold)	toBeRemoved.push_back(scores_offsets[scores_offsets_iterator].second);
		}
		//remove overlapping boxes, i hate this method but idk if theres a faster way to remove values
		//std::printf("num to be removed %d\n",toBeRemoved.size());
		for(unsigned int remover_index = 0; remover_index < toBeRemoved.size();remover_index++){
			for(unsigned int remover_iterator = 0; remover_iterator < scores_offsets.size(); remover_iterator++){
				if(toBeRemoved[remover_index]==scores_offsets[remover_iterator].second)scores_offsets.erase(scores_offsets.begin()+remover_iterator);
			}
		}
		//remove reference pair
		scores_offsets.pop_back();
	}
	return picked;
}

}  // namespace isaac
