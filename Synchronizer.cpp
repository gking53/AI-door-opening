#include "apps/eclipseworkspace/facial/Synchronizer.hpp"

#include <string>
#include <iostream>
#include <chrono>
#include <ctime>

#include "engine/gems/image/utils.hpp"
#include "engine/gems/image/conversions.hpp"

namespace isaac {

void Synchronizer::start() {
	tickOnMessage(rx_rgb());
}

void Synchronizer::tick() {
	//get all inputs
	ImageConstView3ub const_rgb;
	FromProto(rx_rgb().getProto(), rx_rgb().buffers(), const_rgb);
	Image3ub rgb(const_rgb.rows(),const_rgb.cols());
	Image3ub sync_rgb(const_rgb.rows(),const_rgb.cols());
	Copy(const_rgb,rgb);
	Copy(const_rgb,sync_rgb);
	ToProto(std::move(sync_rgb), tx_sync_rgb().initProto(), tx_sync_rgb().buffers());

	while(!rx_depth().available());
	while(!rx_intrinsics().available());
	ImageConstView1f const_depth;
	FromProto(rx_depth().getProto(), rx_depth().buffers(), const_depth);
	Image1f depth(const_depth.rows(),const_depth.cols());
	Copy(const_depth,depth);
	ToProto(std::move(depth), tx_sync_depth().initProto(), tx_sync_depth().buffers());
	auto intrinsics_reader = rx_intrinsics().getProto();
	auto intrinsics_builder = tx_sync_intrinsics().initProto();
	intrinsics_builder.setPinhole(intrinsics_reader.getPinhole());
	//intrinsics_builder.setDistortion(intrinsics_reader.getDistortion());
	//geometry::PinholeD pinhole;
	//FromProto(intrinsics_reader,pinhole);
	//printf("rows %d, cols %d, focal: %f, %f, center %f %f\n",pinhole.dimensions[0],pinhole.dimensions[1],pinhole.focal[0],pinhole.focal[1],pinhole.center[0],pinhole.center[1]);
	/*const int64_t offset = getTickTimestamp()/2000000;
	Image1f buffer_depth(480, 640);
	for (int i = 0; i < buffer_depth.rows(); i++) {
		const int64_t ii = static_cast<int64_t>(i) + offset;
		for (int j = 0; j < buffer_depth.cols(); j++) {
			const int64_t jjp = static_cast<int64_t>(j) + offset;
			const int64_t jjm = static_cast<int64_t>(j) - offset;
			const unsigned char ci = static_cast<unsigned char>(ii);
			const unsigned char cjp = static_cast<unsigned char>(jjp);
			const unsigned char cjm = static_cast<unsigned char>(jjm);
			const unsigned char cx = static_cast<unsigned char>(i * j);
			const float q
			          = 2.0f*static_cast<float>(j) / static_cast<float>(buffer_depth.cols()) - 1.0f;
			buffer_depth(i, j) = 1.5f + 5.f*q*q
				+ 0.5f*std::cos(0.05f*static_cast<float>(ii))
		    	+ 0.5f*std::cos(0.05f*static_cast<float>(jjp));
		}
	}
	ToProto(std::move(buffer_depth), tx_sync_depth().initProto(), tx_sync_depth().buffers());
	tx_sync_depth().publish();
	*/

	//generate detections
	ToProto(std::move(rgb),tx_detections_rgb().initProto(), tx_detections_rgb().buffers());
	tx_detections_rgb().publish();

	auto start = std::chrono::system_clock::now();
	while(!rx_detections().available()){
		auto end = std::chrono::system_clock::now();
		std::chrono::duration<double> elapsed_seconds = end-start;
		if(elapsed_seconds.count() > 1.0){
			return;
		}
	}
	const auto& detections2_reader = rx_detections().getProto();
	auto detections_proto = tx_sync_detections().initProto();

	detections_proto.setPredictions(detections2_reader.getPredictions());
	detections_proto.setBoundingBoxes(detections2_reader.getBoundingBoxes());

	//const int64_t acqtime = rx_depth().pubtime();
	const int64_t acqtime = getTickTime();
	tx_sync_rgb().publish(acqtime);
	tx_sync_detections().publish(acqtime);
	tx_sync_depth().publish(acqtime);
	tx_sync_intrinsics().publish(acqtime);
}
}
