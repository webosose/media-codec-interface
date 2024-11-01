#ifndef SRC_RESOURCE_RESOURCE_WRAPPER_STDLIBCPP_RESOURCE_MANAGER_CLIENT_H_
#define SRC_RESOURCE_RESOURCE_WRAPPER_STDLIBCPP_RESOURCE_MANAGER_CLIENT_H_

extern "C" {

typedef bool (*funcOnPolicyAction)(void*,
                                   const char*,
                                   const char*,
                                   const char*,
                                   const char*,
                                   const char*);
typedef void (*PolicyActionCallbackInternal)(const char*,
                                             const char*,
                                             const char*,
                                             const char*,
                                             const char*);
void* CreateResourceManagerClient(void* wrapper);
void* CreateResourceManagerClientWithConnectionId(void*, const char*);
void DestroyResourceManagerClient(void* ptr);

bool ResourceManagerClientRegisterPipeline(void* ptr, const char* type);
const char* ResourceManagerClientAcquire(void* ptr,
                                         const char* payload,
                                         bool& result);
bool ResourceManagerClientRelease(void* ptr, const char* resources);
bool ResourceManagerClientNotifyForeground(void* ptr);
bool ResourceManagerClientNotifyBackground(void* ptr);
const char* ResourceManagerClientGetConnectionID(void* ptr);
void ResourceManagerClientRegisterPolicyActionHandler(
    void* ptr,
    funcOnPolicyAction callback);
}
#endif  // SRC_RESOURCE_RESOURCE_WRAPPER_STDLIBCPP_RESOURCE_MANAGER_CLIENT_H_
