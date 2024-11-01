#ifndef SRC_RESOURCE_RESOURCE_RESOURCE_WRAPPER_CALCULATOR_TYPE_CONVERTER_H_
#define SRC_RESOURCE_RESOURCE_RESOURCE_WRAPPER_CALCULATOR_TYPE_CONVERTER_H_

#include <cstdint>
#include <string>
#include <vector>

#include <res_calculator.h>

namespace cpplib_glue {
namespace mrc {

class Resource {
 public:
  Resource(const std::string &type_str, int32_t quantity_num);
  virtual ~Resource();

  std::string type;
  int32_t quantity;
};

typedef std::vector<Resource> ResourceList;
typedef std::vector<ResourceList> ResourceListOptions;

void concatResourceList(ResourceList *dst, const ResourceList *src);
void concatResourceListOptions(ResourceListOptions* dst,
                               const ResourceListOptions* src);

}  //  namespace mrc
}  //  namespace cpplib_glue

struct resource_list_options_wrapper_t {
  cpplib_glue::mrc::ResourceListOptions internal_options_;
};

void* resourceListOptionsWrapperTConstructor();
void resourceListOptionsTAddResourceListT(void* rlow);
void resourceListTAddResourceT(void* rlow, const char* type, int32_t quantity);

#endif  // SRC_RESOURCE_RESOURCE_RESOURCE_WRAPPER_CALCULATOR_TYPE_CONVERTER_H_
