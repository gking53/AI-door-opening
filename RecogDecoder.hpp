#pragma once

#include "engine/alice/alice_codelet.hpp"
#include "messages/tensor.hpp"
#include "engine/core/tensor/tensor.hpp"
#include "engine/gems/tensor/utils.hpp"

namespace isaac {

class RecogDecoder : public alice::Codelet {
 public:
  void start() override;
  void tick() override;

  Tensor2f past_embeddings;
  bool no_past;

  ISAAC_PROTO_RX(TensorProto, embeddings);
  ISAAC_PROTO_RX(TensorProto, input);
  ISAAC_PROTO_TX(TensorProto, output);

 private:
};

}  // namespace isaac

ISAAC_ALICE_REGISTER_CODELET(isaac::RecogDecoder);
