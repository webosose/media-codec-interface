#include "resource_manager_client.h"
#include "resource_manager_client_impl.h"

typedef uMediaServer::ResourceManagerClient ResourceManagerClient;
typedef stdlibcpp::ResourceManagerClientImpl ResourceManagerClientImpl;

void* CreateResourceManagerClient(void* wrapper) {
  return new ResourceManagerClientImpl(wrapper);
}

void* CreateResourceManagerClientWithConnectionId(void* wrapper,
                                                  const char* id) {
  return new ResourceManagerClientImpl(wrapper, id);
}

void DestroyResourceManagerClient(void* ptr) {
  ResourceManagerClientImpl* rmc = static_cast<ResourceManagerClientImpl*>(ptr);
  if (rmc)
    delete rmc;
}

bool ResourceManagerClientRegisterPipeline(void* ptr, const char* type) {
  ResourceManagerClientImpl* rmc = static_cast<ResourceManagerClientImpl*>(ptr);
  return rmc->registerPipeline(type);
}

const char* ResourceManagerClientAcquire(void* ptr,
                                         const char* payload,
                                         bool& result) {
  ResourceManagerClientImpl* rmc = static_cast<ResourceManagerClientImpl*>(ptr);
  static std::string reply;
  result = rmc->acquire(payload, reply);
  return reply.c_str();
}

bool ResourceManagerClientRelease(void* ptr, const char* resources) {
  ResourceManagerClientImpl* rmc = static_cast<ResourceManagerClientImpl*>(ptr);
  return rmc->release(resources);
}

bool ResourceManagerClientNotifyForeground(void* ptr) {
  ResourceManagerClientImpl* rmc = static_cast<ResourceManagerClientImpl*>(ptr);
  return rmc->notifyForeground();
}

bool ResourceManagerClientNotifyBackground(void* ptr) {
  ResourceManagerClientImpl* rmc = static_cast<ResourceManagerClientImpl*>(ptr);
  return rmc->notifyBackground();
}

const char* ResourceManagerClientGetConnectionID(void* ptr) {
  ResourceManagerClientImpl* rmc = static_cast<ResourceManagerClientImpl*>(ptr);
  static std::string id = rmc->getConnectionID();
  return id.c_str();
}

void ResourceManagerClientRegisterPolicyActionHandler(
    void* ptr,
    funcOnPolicyAction callback) {
  ResourceManagerClientImpl* rmc = static_cast<ResourceManagerClientImpl*>(ptr);
  rmc->RegisterHandlerForPolicy(callback);
}
