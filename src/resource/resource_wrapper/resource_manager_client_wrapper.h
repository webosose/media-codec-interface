#ifndef SRC_RESOURCE_RESOURCE_WRAPPER_RESOURCE_MANAGER_CLIENT_WRAPPER_H_
#define SRC_RESOURCE_RESOURCE_WRAPPER_RESOURCE_MANAGER_CLIENT_WRAPPER_H_
#if !defined(ENABLE_WRAPPER)
#include <ResourceManagerClient.h>
#else
#include <string>

namespace cpplib_glue {
namespace uMediaServer {
typedef std::function<
    bool(const char*, const char*, const char*, const char*, const char*)>
    ResourcePolicyActionCallback;

class ResourceManagerClientInternal {
 public:
  // used by external clients
  ResourceManagerClientInternal();

  ResourceManagerClientInternal(const std::string& connection_id);
  virtual ~ResourceManagerClientInternal();
  bool registerPipeline(std::string type,
                        const std::string& app_id = std::string(),
                        bool reserved = false);
  bool acquire(const std::string& payload, std::string& response);
  bool release(std::string resources);
  bool notifyForeground();
  bool notifyBackground();
  const char* getConnectionID();
  void registerPolicyActionHandler(ResourcePolicyActionCallback callback);

  bool policyActionHandler(const char* action,
                           const char* resources,
                           const char* requestor_type,
                           const char* requestor_name,
                           const char* connection_id);

 private:
  void* resource_manager_client_;
  ResourcePolicyActionCallback policy_action_callback_;
};

class ResourceManagerClient
    : public cpplib_glue::uMediaServer::ResourceManagerClientInternal {
 public:
  ResourceManagerClient() : ResourceManagerClientInternal() {}

  ResourceManagerClient(const std::string& connection_id)
      : ResourceManagerClientInternal(connection_id) {}

  virtual ~ResourceManagerClient() {}
};

}  //  namespace uMediaServer
}  //  namespace cpplib_glue

namespace uMediaServer = cpplib_glue::uMediaServer;

#endif  // !defined(USE_MEDIA_WRAPPER)
#endif  // SRC_RESOURCE_RESOURCE_WRAPPER_RESOURCE_MANAGER_CLIENT_WRAPPER_H_
