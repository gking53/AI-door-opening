#pragma once

#include "apps/eclipseworkspace/facial/TrackerObject.hpp"
#include "apps/eclipseworkspace/facial/UdpClient.hpp"
#include "engine/alice/alice_codelet.hpp"
#include "messages/detections.capnp.h"
#include <vector>

namespace isaac {

class Tracker : public alice::Codelet {
 public:
  void start() override;
  void tick() override;

  ISAAC_PROTO_RX(Detections3Proto, enterance_detections);
  ISAAC_PROTO_RX(Detections3Proto, exit_detections);

  //squared distance thresh, to save a sqrt call
  ISAAC_PARAM(double, distance_threshold, 0.01)
  ISAAC_PARAM(int, frames_threshold, 3)

 private:
  udp_client* client;
  std::vector<TrackerObject> tracked_objects;

  bool DoorState;

  int num_people;
  int trackObject(Vector3d location);
  void removeUntracked();

};

}  // namespace isaac

ISAAC_ALICE_REGISTER_CODELET(isaac::Tracker);
