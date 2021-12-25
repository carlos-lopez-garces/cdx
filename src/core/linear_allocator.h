#pragma once

#include "gpu_resource.h"

enum LinearAllocatorType {
  kInvalidAllocator = -1,
  // TODO: describe.
  kGPUExclusive = 0,
  kCPUWritable = 1,
  kNumAllocatorTypes
};