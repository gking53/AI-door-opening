#pragma once

#include "engine/alice/alice_codelet.hpp"
#include "messages/tensor.hpp"
#include "engine/core/tensor/tensor.hpp"
#include "engine/gems/tensor/utils.hpp"

#include "google/cloud/storage/client.h"

namespace isaac {

class CloudStorage : public alice::Codelet {
 public:
  void start() override;
  void tick() override;

  ISAAC_PROTO_RX(TensorProto, embeddings);

  ISAAC_PARAM(std::string, file_name)
  ISAAC_PARAM(std::string, bucket_name)

 private:
  google::cloud::StatusOr<google::cloud::storage::Client> client;
};

}  // namespace isaac

ISAAC_ALICE_REGISTER_CODELET(isaac::CloudStorage);
