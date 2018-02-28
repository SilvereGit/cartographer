/*
 * Copyright 2017 The Cartographer Authors
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#ifndef CARTOGRAPHER_GRPC_HANDLERS_ADD_TRAJECTORY_HANDLER_H
#define CARTOGRAPHER_GRPC_HANDLERS_ADD_TRAJECTORY_HANDLER_H

#include "cartographer_grpc/framework/rpc_handler.h"
#include "cartographer_grpc/proto/map_builder_service.pb.h"

namespace cartographer_grpc {
namespace handlers {

class AddTrajectoryHandler
    : public framework::RpcHandler<proto::AddTrajectoryRequest,
                                   proto::AddTrajectoryResponse> {
 public:
  std::string method_name() const override {
    return "/cartographer_grpc.proto.MapBuilderService/AddTrajectory";
  }
  void OnRequest(const proto::AddTrajectoryRequest& request) override;
};

}  // namespace handlers
}  // namespace cartographer_grpc

#endif  // CARTOGRAPHER_GRPC_HANDLERS_ADD_TRAJECTORY_HANDLER_H
