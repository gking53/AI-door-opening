#pragma once

#include "messages/math.hpp"

namespace isaac {

	class TrackerObject {
	public:

		enum Location {Outside, Inside, Inside_Door, Outside_Door};

		//x y z position of object in meters
		Vector3d location;
		//change in x y z position in meters from last tick to current. Because constant sampling rate, 2 multiplication operations can be saved by storing it this way
		Vector3d velocity;
		//ID of area the object is currently
		Location area;
		//how many frames the object has been missing
		int missing_frames;
		//was the object already tracked
		bool tracked;

		TrackerObject(Vector3d starting_location){
			velocity = Vector3d(0,0,0);
			location = starting_location;
			if(location(0)>0){
				area=Inside; //inside
			}
			else{
				area=Outside; //outside
			}
			missing_frames = 0;
			tracked = true;
		}

		int updateLocation(Vector3d new_location);
	};
}
