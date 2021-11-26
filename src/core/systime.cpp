#include "pch.h"
#include "SystemTime.h"

double SystemTime::sm_CpuTickDelta = 0.0;

void SystemTime::Initialize(void)
{
  LARGE_INTEGER frequency;
  ASSERT(TRUE == QueryPerformanceFrequency(&frequency), "Unable to query performance counter frequency");
  sm_CpuTickDelta = 1.0 / static_cast<double>(frequency.QuadPart);
}

int64_t SystemTime::GetCurrentTick(void)
{
  LARGE_INTEGER currentTick;
  ASSERT(TRUE == QueryPerformanceCounter(&currentTick), "Unable to query performance counter value");
  return static_cast<int64_t>(currentTick.QuadPart);
}

void SystemTime::BusyLoopSleep(float SleepTime)
{
  int64_t finalTick = (int64_t)((double)SleepTime / sm_CpuTickDelta) + GetCurrentTick();
  while (GetCurrentTick() < finalTick);
}