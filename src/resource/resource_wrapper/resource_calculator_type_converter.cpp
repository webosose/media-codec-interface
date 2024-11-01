#include "resource_calculator_type_converter.h"

void* resourceListOptionsWrapperTConstructor() {
  return new resource_list_options_wrapper_t;
}

void resourceListOptionsTAddResourceListT(void* rlow) {
  resource_list_options_wrapper_t* resource_list_options_wrapper =
      static_cast<resource_list_options_wrapper_t*>(rlow);
  resource_list_options_wrapper->internal_options_.push_back({});
}

void resourceListTAddResourceT(void* rlow, const char* type, int32_t quantity) {
  resource_list_options_wrapper_t* resource_list_options_wrapper =
      static_cast<resource_list_options_wrapper_t*>(rlow);
  cpplib_glue::mrc::ResourceList& resource_list =
      resource_list_options_wrapper->internal_options_.back();
  resource_list.push_back(cpplib_glue::mrc::Resource(type, quantity));
}
