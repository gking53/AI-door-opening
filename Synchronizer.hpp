#pragma once

#include "engine/alice/alice_codelet.hpp"
#include "messages/detections.capnp.h"
#include "messages/image.capnp.h"
#include "messages/camera.capnp.h"
#include "messages/image.hpp"
#include "messages/camera.hpp"

namespace isaac {

class Synchronizer : public alice::Codelet {
 public:
  void start() override;
  void tick() override;

  ISAAC_PROTO_RX(ImageProto, rgb);
  ISAAC_PROTO_RX(ImageProto, depth);
  ISAAC_PROTO_RX(CameraIntrinsicsProto, intrinsics);
  ISAAC_PROTO_RX(Detections2Proto, detections);

  ISAAC_PROTO_TX(ImageProto, detections_rgb);
  ISAAC_PROTO_TX(ImageProto, sync_depth);
  ISAAC_PROTO_TX(ImageProto, sync_rgb);
  ISAAC_PROTO_TX(Detections2Proto, sync_detections);
  ISAAC_PROTO_TX(CameraIntrinsicsProto, sync_intrinsics);

 private:
};

}  // namespace isaac

ISAAC_ALICE_REGISTER_CODELET(isaac::Synchronizer);
