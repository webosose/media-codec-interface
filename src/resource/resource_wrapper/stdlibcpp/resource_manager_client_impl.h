#ifndef SRC_RESOURCE_RESOURCE_WRAPPER_STDLIBCPP_RESOURCE_MANAGER_CLIENT_IMPL_H_
#define SRC_RESOURCE_RESOURCE_WRAPPER_STDLIBCPP_RESOURCE_MANAGER_CLIENT_IMPL_H_

#include <ResourceManagerClient.h>

#include "resource_manager_client.h"

namespace stdlibcpp {
class ResourceManagerClientImpl : public uMediaServer::ResourceManagerClient {
 public:
  ResourceManagerClientImpl(void* wrapper);
  ResourceManagerClientImpl(void* wrapper, const char* id);

  void RegisterHandlerForPolicy(funcOnPolicyAction callback);

 private:
  bool PolicyActionHandler(const char* action,
                           const char* resources,
                           const char* requestor_type,
                           const char* requestor_name,
                           const char* connection_id);

 private:
  funcOnPolicyAction policy_action_callback_;
  void* wrapper_;
};

}  // namespace stdlibcpp

#endif  // SRC_RESOURCE_RESOURCE_WRAPPER_STDLIBCPP_RESOURCE_MANAGER_CLIENT_IMPL_H_
