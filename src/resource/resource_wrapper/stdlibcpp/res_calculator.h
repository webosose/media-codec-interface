#ifndef SRC_RESOURCE_RESOURCE_WRAPPER_STDLIBCPP_RES_CALCULATOR_H_
#define SRC_RESOURCE_RESOURCE_WRAPPER_STDLIBCPP_RES_CALCULATOR_H_

#include <stdint.h>

extern "C" {
typedef void* (*funcResourceListOptionsWrapperTConstructor)();
typedef void (*funcResourceListTAddResourceT)(void* rlow,
                                              const char* type,
                                              int32_t quantity);
typedef void (*funcResourceListOptionsTAddResourceListT)(void* rlow);

typedef struct {
  funcResourceListOptionsWrapperTConstructor
      resourceListOptionsWrapperTConstructorFunc;
  funcResourceListTAddResourceT resourceListTAddResourceTFunc;
  funcResourceListOptionsTAddResourceListT
      resourceListOptionsTAddResourceListTFunc;
} ResourceCalculatorVirtualFuncs;

void* CreateResourceCalculator();
void DestroyResourceCalculator(void* wrapper);

void* ResourceCalculatorCalcVdecResourceOptions(
    void* that,
    const ResourceCalculatorVirtualFuncs& funcs,
    int32_t codecs,
    int32_t width,
    int32_t height,
    int32_t frame_rate_data,
    int scan_type_data,
    int type_data_3d);

void* ResourceCalculatorCalcVencResourceOptions(
    void* that,
    const ResourceCalculatorVirtualFuncs& funcs,
    int32_t codecs,
    int32_t width,
    int32_t height,
    int32_t frame_rate);
}
#endif  // SRC_RESOURCE_RESOURCE_WRAPPER_STDLIBCPP_RES_CALCULATOR_H_
