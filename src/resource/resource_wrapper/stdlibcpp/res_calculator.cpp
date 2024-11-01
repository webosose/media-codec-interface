#include "res_calculator.h"

#include <media-resource-calculator/resource_calculator.h>

typedef mrc::ResourceCalculator MRC;

void* CreateResourceCalculator() {
  return MRC::create();
}

void DestroyResourceCalculator(void* ptr) {
  MRC* mrc = static_cast<MRC*>(ptr);
  if (mrc)
    delete mrc;
}

void* ResourceCalculatorCalcVdecResourceOptions(
    void* that,
    const ResourceCalculatorVirtualFuncs& funcs,
    int32_t codecs,
    int32_t width,
    int32_t height,
    int32_t frame_rate_data,
    int scan_type_data,
    int type_data_3d) {
  MRC* client = static_cast<MRC*>(that);
  mrc::ResourceListOptions resource_list_options =
      client->calcVdecResourceOptions(
          static_cast<MRC::VideoCodecs>(codecs), width, height, frame_rate_data,
          static_cast<MRC::ScanType>(scan_type_data),
          static_cast<MRC::_3DType>(type_data_3d));

  void* rlow = funcs.resourceListOptionsWrapperTConstructorFunc();
  for (auto& rlo : resource_list_options) {
    funcs.resourceListOptionsTAddResourceListTFunc(rlow);
    for (auto& rl : rlo) {
      funcs.resourceListTAddResourceTFunc(rlow, rl.type.c_str(), rl.quantity);
    }
  }
  return rlow;
}

void* ResourceCalculatorCalcVencResourceOptions(
    void* that,
    const ResourceCalculatorVirtualFuncs& funcs,
    int32_t codecs,
    int32_t width,
    int32_t height,
    int32_t frame_rate) {
  MRC* client = static_cast<MRC*>(that);
  mrc::ResourceListOptions resource_list_options =
      client->calcVencResourceOptions(static_cast<MRC::VideoCodecs>(codecs),
                                      width, height, frame_rate);

  void* rlow = funcs.resourceListOptionsWrapperTConstructorFunc();
  for (auto& rlo : resource_list_options) {
    funcs.resourceListOptionsTAddResourceListTFunc(rlow);
    for (auto& rl : rlo) {
      funcs.resourceListTAddResourceTFunc(rlow, rl.type.c_str(), rl.quantity);
    }
  }
  return rlow;
}
