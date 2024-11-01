#include "resource_calculator_wrapper.h"

#include <res_calculator.h>

#include <assert.h>
#include <functional>
#include <map>
#include <memory>
#include <string>

#include "base/log.h"

namespace {
ResourceCalculatorVirtualFuncs resource_calculator_funcs_list{
    resourceListOptionsWrapperTConstructor, resourceListTAddResourceT,
    resourceListOptionsTAddResourceListT};
}  // namespace

namespace cpplib_glue {
namespace mrc {

ResourceCalculator* ResourceCalculator::create() {
  return new ResourceCalculator();
}

Resource::Resource(const std::string& type_str, int32_t quantity_num)
    : type(type_str), quantity(quantity_num) {}

Resource::~Resource() {}

void concatResourceList(ResourceList* dst, const ResourceList* src) {
  assert(dst);

  if ((src == nullptr) || src->empty()) {
    return;
  }

  if (dst->empty()) {
    *dst = *src;
    return;
  }

  for (ResourceList::const_iterator is = src->begin(); is != src->end(); ++is) {
    ResourceList::iterator id;
    for (id = dst->begin(); id != dst->end(); ++id) {
      if (0 == id->type.compare(is->type)) {
        if (id->quantity < is->quantity) {
          id->quantity = is->quantity;
        }
        break;
      }
    }
    if (id == dst->end()) {
      dst->push_back(*is);
    }
  }
}

void concatResourceListOptions(ResourceListOptions* dst,
                               const ResourceListOptions* src) {
  if ((src == nullptr) || src->empty()) {
    return;
  }

  if (dst->empty()) {
    *dst = *src;
    return;
  }

  ResourceListOptions newOptions;
  ResourceListOptions::const_iterator i;
  ResourceListOptions::const_iterator j;
  for (i = dst->begin(); i != dst->end(); i++) {
    for (j = src->begin(); j != src->end(); j++) {
      ResourceList res = *i;
      concatResourceList(&res, &*j);
      newOptions.push_back(res);
    }
  }

  *dst = newOptions;
}

ResourceCalculator::ResourceCalculator()
    : resource_calculator_(CreateResourceCalculator()) {}

ResourceCalculator::~ResourceCalculator() {
  DestroyResourceCalculator(resource_calculator_);
}

ResourceListOptions ResourceCalculator::calcVdecResourceOptions(
    VideoCodecs codecs,
    int32_t width,
    int32_t height,
    int32_t frame_rate_data,
    ScanType scan_type_data,
    _3DType type_data_3d) {
  resource_list_options_wrapper_t* rlow =
      static_cast<resource_list_options_wrapper_t*>(
          ResourceCalculatorCalcVdecResourceOptions(
              resource_calculator_, resource_calculator_funcs_list, codecs,
              width, height, frame_rate_data,
              (std::underlying_type<ScanType>::type)scan_type_data,
              (std::underlying_type<_3DType>::type)type_data_3d));
  if (!rlow) {
    MCIL_ERROR_PRINT("ResourceListOption is null from wrapper");
    return ResourceListOptions();
  }

  ResourceListOptions result = rlow->internal_options_;
  delete rlow;
  return result;
}

ResourceListOptions ResourceCalculator::calcVencResourceOptions(
    VideoCodecs codecs,
    int32_t width,
    int32_t height,
    int32_t frame_rate) {
  resource_list_options_wrapper_t* rlow =
      static_cast<resource_list_options_wrapper_t*>(
          ResourceCalculatorCalcVencResourceOptions(
              resource_calculator_, resource_calculator_funcs_list, codecs,
              width, height, frame_rate));
  if (!rlow) {
    MCIL_ERROR_PRINT("ResourceListOption is null from wrapper");
    return ResourceListOptions();
  }

  ResourceListOptions result = rlow->internal_options_;
  delete rlow;
  return result;
}

}  // namespace mrc
}  // namespace cpplib_glue
