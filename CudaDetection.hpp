#pragma once

#include "engine/alice/alice_codelet.hpp"
#include "messages/geometry.hpp"
#include "messages/tensor.hpp"
#include "messages/detections.capnp.h"

namespace isaac {

class CudaDetection : public alice::Codelet {
 public:
  void start() override;
  void tick() override;

  ISAAC_PROTO_RX(TensorProto, scores);
  ISAAC_PROTO_RX(TensorProto, boxes);
  ISAAC_PROTO_TX(Detections2Proto, detections);

  ISAAC_PARAM(double, score_threshold, 0.90)
  ISAAC_PARAM(double, box_threshold, 0.15)
  ISAAC_PARAM(double, cols, 1280)
  ISAAC_PARAM(double, rows, 720)

 private:
  std::vector<std::pair<float,int>> filterBoxes(TensorConstView3f& scores, TensorConstView3f& boxes);
};

}  // namespace isaac

ISAAC_ALICE_REGISTER_CODELET(isaac::CudaDetection);
