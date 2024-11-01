#include "resource_manager_client_wrapper.h"

#include <resource_manager_client.h>

namespace cpplib_glue {
namespace uMediaServer {
ResourceManagerClientInternal::ResourceManagerClientInternal()
    : resource_manager_client_(CreateResourceManagerClient(this)) {}

ResourceManagerClientInternal::ResourceManagerClientInternal(
    const std::string& connectionId)
    : resource_manager_client_(
          CreateResourceManagerClientWithConnectionId(this,
                                                      connectionId.c_str())) {}

ResourceManagerClientInternal::~ResourceManagerClientInternal() {
  DestroyResourceManagerClient(resource_manager_client_);
}

bool ResourceManagerClientInternal::registerPipeline(std::string type,
                                                     const std::string& app_id,
                                                     bool reserved) {
  return ResourceManagerClientRegisterPipeline(resource_manager_client_,
                                               type.c_str());
}

bool ResourceManagerClientInternal::acquire(const std::string& payload,
                                            std::string& response) {
  bool result;
  response = ResourceManagerClientAcquire(resource_manager_client_,
                                          payload.c_str(), result);
  return result;
}

bool ResourceManagerClientInternal::release(std::string resources) {
  return ResourceManagerClientRelease(resource_manager_client_,
                                      resources.c_str());
}

bool ResourceManagerClientInternal::notifyForeground() {
  return ResourceManagerClientNotifyForeground(resource_manager_client_);
}

bool ResourceManagerClientInternal::notifyBackground() {
  return ResourceManagerClientNotifyBackground(resource_manager_client_);
}

const char* ResourceManagerClientInternal::getConnectionID() {
  return ResourceManagerClientGetConnectionID(resource_manager_client_);
}

bool OnPolicyActionHandler(void* that,
                           const char* action,
                           const char* resources,
                           const char* requestor_type,
                           const char* requestor_name,
                           const char* connection_id) {
  cpplib_glue::uMediaServer::ResourceManagerClientInternal* client =
      static_cast<cpplib_glue::uMediaServer::ResourceManagerClientInternal*>(
          that);
  return client->policyActionHandler(action, resources, requestor_type,
                                     requestor_name, connection_id);
}

void ResourceManagerClientInternal::registerPolicyActionHandler(
    ResourcePolicyActionCallback callback) {
  policy_action_callback_ = callback;
  ResourceManagerClientRegisterPolicyActionHandler(resource_manager_client_,
                                                   &OnPolicyActionHandler);
}

bool ResourceManagerClientInternal::policyActionHandler(
    const char* action,
    const char* resources,
    const char* requestor_type,
    const char* requestor_name,
    const char* connection_id) {
  return policy_action_callback_(action, resources, requestor_type,
                                 requestor_name, connection_id);
}

}  // namespace uMediaServer
}  // namespace cpplib_glue
