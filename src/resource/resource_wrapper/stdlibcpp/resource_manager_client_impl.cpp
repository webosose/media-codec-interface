#include "resource_manager_client_impl.h"

namespace stdlibcpp {
ResourceManagerClientImpl::ResourceManagerClientImpl(void* wrapper)
    : wrapper_(wrapper) {}

ResourceManagerClientImpl::ResourceManagerClientImpl(void* wrapper,
                                                     const char* id)
    : uMediaServer::ResourceManagerClient(id), wrapper_(wrapper) {}

void ResourceManagerClientImpl::RegisterHandlerForPolicy(
    funcOnPolicyAction policy_action_callback) {
  policy_action_callback_ = policy_action_callback;
  registerPolicyActionHandler(std::bind(
      &ResourceManagerClientImpl::PolicyActionHandler, this,
      std::placeholders::_1, std::placeholders::_2, std::placeholders::_3,
      std::placeholders::_4, std::placeholders::_5));
}

bool ResourceManagerClientImpl::PolicyActionHandler(const char* action,
                                                    const char* resources,
                                                    const char* requestor_type,
                                                    const char* requestor_name,
                                                    const char* connection_id) {
  return policy_action_callback_(wrapper_, action, resources, requestor_type,
                                 requestor_name, connection_id);
}

}  // namespace stdlibcpp
