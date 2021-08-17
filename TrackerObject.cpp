#include "apps/eclipseworkspace/facial/TrackerObject.hpp"

namespace isaac {

//take new location, calculate velocity and update parameters
int TrackerObject::updateLocation(Vector3d new_location){
	//complementary filter to smooth results
	this->velocity = .85*(new_location-location)+.15*this->velocity;
	this->location = new_location;
	this->tracked = true;
	this->missing_frames = 0;
	/*
	 * 0x----
	 * left most is inside door flag
	 * next is outside door flag
	 * next is left store flag
	 * last is enter store flag
	 */

	int flag = 0;
	if(location(1)>2 && location(1)<(2+.25)){
		//if going from outside to inside
		if(area==Outside_Door){
			area = Inside_Door;
			//person has entered the store
			flag = 0x1001;
		}
		else{
			area=Inside_Door;
			flag = 0x1000;
		}

	}
	else if(location(1)<=2 && location(1)>(2-.25)){
		if(area==Inside_Door){
			area=Outside_Door;
			flag = 0x0110;
		}
		else{
			area=Outside_Door;
			flag = 0x0100;
		}
	}
	else if(location(1)<=(2-.25)){
		area=Outside;
	}
	else if(location(1)>=(2+.25)){
		area=Inside;
	}
	return flag;
}
};
