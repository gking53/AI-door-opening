/*
 * need object to store tracking info, should be a class
 *
 * class tracker:
 * vector<tracked_object> objects
 * int persons_in_store
 *
 * function should be fed all 3space locations of boxes and should update list of objects
 *
 * class tracked_object:
 * vector3d location //stores 3space location of object
 * vector3d velocity //stores all veloty vectors of object
 * int location //which area the object is in
 * int missing_frames //how many frames it has been missing so far
 * bool updated //has location be updated this loop, set to false on start of
 *
 * velocity should be updated based on previous location when new location is fed, update fields using function that takes new location and returns if it is in store or not
 *
 * to check new position, take new location vector in 3space, for each tracked object, generate predicted new location using velocity matrix and location matrix, then check distance by subtracting predicted
 * location from actual location and checking the norm of it, if norm is less than min distance times missing frames, update the location and velocity, and remove the detection from the list, if no track
 * is found add new track object. Afterwards update all tracked objects missing frames, if any are at 3, remove them from the objects vector
 */

#include "apps/eclipseworkspace/facial/Tracker.hpp"

#include <iostream>

#include "engine/core/math/types.hpp"
#include "engine/gems/geometry/n_cuboid.hpp"

namespace isaac {

void Tracker::start() {
	num_people = 0;
	DoorState = false;
	this->client = new udp_client("100.64.119.131",8001);
	tickOnMessage(rx_enterance_detections());
}

void Tracker::tick() {
	//wait for exit detections proto
	auto start = std::chrono::system_clock::now();
	while(!rx_exit_detections().available()){
		auto end = std::chrono::system_clock::now();
		std::chrono::duration<double> elapsed_seconds = end-start;
		if(elapsed_seconds.count() > 1.0){
			return;
		}
	}
	//get both transforms for detections to world
	Pose3d enterance_transform = node()->pose().tryGetLatest("world", "enterance_camera").value().first;
	Pose3d exit_transform = node()->pose().tryGetLatest("world", "exit_camera").value().first;
	//grab all detection protos and read poses into memory
	const auto& enterance_detections_reader = rx_enterance_detections().getProto();
	const auto& enterance_pose_reader = enterance_detections_reader.getPoses();
	std::vector<Vector3d> enterance_poses;
	for(size_t i=0;i<enterance_pose_reader.size();++i){
		enterance_poses.push_back((FromProto(enterance_pose_reader[i])*enterance_transform).translation);
	}
	const auto& exit_detections_reader = rx_exit_detections().getProto();
	const auto& exit_pose_reader = exit_detections_reader.getPoses();
	std::vector<Vector3d> exit_poses;
	for(size_t i=0;i<exit_pose_reader.size();++i){
		Vector3d temp;
		temp(0) = ((FromProto(exit_pose_reader[i])).translation(0)*-1)-exit_transform.translation(0);
		temp(1) = ((FromProto(exit_pose_reader[i])).translation(1)*-1)-exit_transform.translation(1);
		temp(2) = (FromProto(exit_pose_reader[i])).translation(2)-exit_transform.translation(2);
		exit_poses.push_back(temp);
	}
	for(size_t i =0;i<this->tracked_objects.size();++i){
		this->tracked_objects[i].tracked = false;
	}
	//check for overlaps
	std::vector<Vector3d> poses;
	if(enterance_poses.size()!=0){
		poses = enterance_poses;
		for(size_t i = 0;i<exit_poses.size();++i){
			for(size_t j = 0;j<enterance_poses.size();++j){
				//if detection isnt overlapping by 20 cm, put it at the end of the pose list
				if((exit_poses[i]-enterance_poses[j]).squaredNorm()>0.25)poses.push_back(exit_poses[i]);
			}
		}
	}
	else if(exit_poses.size()!=0){
		poses = exit_poses;
		for(size_t i = 0;i<exit_poses.size();++i){
			for(size_t j = 0;j<enterance_poses.size();++j){
				//if detection isnt overlapping by 20 cm, put it at the end of the pose list
				if((exit_poses[i]-enterance_poses[j]).squaredNorm()>0.25)poses.push_back(enterance_poses[i]);
			}
		}
	}
	//get tracking
	/*
	 * 0x----
	 * left most is inside door flag
	 * next is outside door flag
	 * next is left store flag
	 * last is enter store flag
	 */
	int flag;
	bool open_door = false;
	for(size_t i = 0;i<poses.size();++i){
		flag = trackObject(poses[i]);
		if((flag & 0x1000) == 0x1000){
			open_door = true;
			if((flag & 0x0001) == 0x0001){
				num_people+=1;
			}
		}
		else if((flag & 0x0100) == 0x0100){
			if(num_people<1){
				open_door=true;
			}
			else if((flag & 0x0010) == 0x0010){
				num_people+=-1;
			}
		}
	}
	if(open_door && DoorState==false){
		std::string message = "open";
		this->client->send(message.c_str(), message.size());
		DoorState = true;
		printf("opened the door\n");
	}
	else if(!open_door && DoorState==true){
		std::string message = "close";
		this->client->send(message.c_str(), message.size());
		DoorState = false;
		printf("closed the door\n");
	}

	removeUntracked();
	if(!this->tracked_objects.empty()){
		show("area",(int)this->tracked_objects[0].area);
		//printf("x y z: %f %f %f\n",this->tracked_objects[0].location(0),this->tracked_objects[0].location(1),this->tracked_objects[0].location(2));
		show("velocity x",this->tracked_objects[0].velocity(0));
		show("velocity y",this->tracked_objects[0].velocity(1));
		show("velocity z",this->tracked_objects[0].velocity(2));
	}

	show("number of people in store",num_people);
}

int Tracker::trackObject(Vector3d location){
	Vector3d estimated_distance;
	for(size_t i =0;i<this->tracked_objects.size();++i){
		//if its already been tracked, skip it
		if(this->tracked_objects[i].tracked==true) continue;
		//position estimated from previous tracked position with estimated distanced traveled added, subtract current object position in prep for norm distance
		estimated_distance = this->tracked_objects[i].location + this->tracked_objects[i].velocity*this->tracked_objects[i].missing_frames - location;
		//if track is within threshold distance
		if(estimated_distance.squaredNorm()<get_distance_threshold()){
			return this->tracked_objects[i].updateLocation(location);
		}
	}
	//if you get here, no track was found and a new object should be added
	this->tracked_objects.push_back(TrackerObject(location));
	return 0;
}

void Tracker::removeUntracked(){
	if(this->tracked_objects.empty()) return;
	std::vector<TrackerObject>::iterator it = this->tracked_objects.begin();
	while(it != this->tracked_objects.end()){
		if(it->tracked == false){
			if(it->missing_frames>get_frames_threshold()){
				tracked_objects.erase(it);
				continue;
			}
			else{
				it->missing_frames++;
			}
		}
		++it;
	}
}

}  // namespace isaac
