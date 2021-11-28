#pragma once

#include "cdx.h"

// Intrinsic functions are built into the compiler (vc++ in this case), as
// opposed to being exported from some library, target a particular architecture
// (x64, SSE4.2 in this case), and are usually highly efficient.
#pragma intrinsic(_mm_crc32_u32, _mm_crc32_u64)

namespace Utils
{
#ifdef _CONSOLE
  inline void Print(const char* msg) { printf("%s", msg); }
  inline void Print(const wchar_t* msg) { wprintf(L"%ws", msg); }
#else
  inline void Print(const char* msg) { OutputDebugStringA(msg); }
  inline void Print(const wchar_t* msg) { OutputDebugString(msg); }
#endif

  inline void Printf(const char* format, ...)
  {
    char buffer[256];
    va_list ap;
    va_start(ap, format);
    vsprintf_s(buffer, 256, format, ap);
    va_end(ap);
    Print(buffer);
  }

  inline void Printf(const wchar_t* format, ...)
  {
    wchar_t buffer[256];
    va_list ap;
    va_start(ap, format);
    vswprintf(buffer, 256, format, ap);
    va_end(ap);
    Print(buffer);
  }

#ifndef RELEASE
  inline void PrintSubMessage(const char* format, ...)
  {
    Print("--> ");
    char buffer[256];
    va_list ap;
    va_start(ap, format);
    vsprintf_s(buffer, 256, format, ap);
    va_end(ap);
    Print(buffer);
    Print("\n");
  }
  inline void PrintSubMessage(const wchar_t* format, ...)
  {
    Print("--> ");
    wchar_t buffer[256];
    va_list ap;
    va_start(ap, format);
    vswprintf(buffer, 256, format, ap);
    va_end(ap);
    Print(buffer);
    Print("\n");
  }
  inline void PrintSubMessage(void) {}
#endif

  std::wstring UTF8ToWideString(const std::string& str);
  std::string WideStringToUTF8(const std::wstring& wstr);
  std::string ToLower(const std::string& str);
  std::wstring ToLower(const std::wstring& str);
  std::string GetBasePath(const std::string& str);
  std::wstring GetBasePath(const std::wstring& str);
  std::string RemoveBasePath(const std::string& str);
  std::wstring RemoveBasePath(const std::wstring& str);
  std::string GetFileExtension(const std::string& str);
  std::wstring GetFileExtension(const std::wstring& str);
  std::string RemoveExtension(const std::string& str);
  std::wstring RemoveExtension(const std::wstring& str);

  template <typename T> __forceinline T alignDownWithMask(T value, size_t mask) {
    // Let alignment = 8 = 1000b. Then mask = alignment-1 = 7 = 0111b. Then ~mask = [...1]1000b.
    // Now let value = 13 = 01101b, for example. Then value & ~mask = 8 = 01000b.
    // Therefore, value & ~mask clears the 3 least-significant bits of value and rounds
    // 13 down to 8.
    return (T)((size_t)value & ~mask);
  }

  template <typename T> __forceinline T alignUpWithMask(T value, size_t mask) {
    // Let alignment = 8 = 1000b. Then mask = alignment-1 = 7 = 0111b. Then ~mask = [...1]1000b.
    // Now let value = 13 = 01101b, for example. Then value + mask = 20 = 10100b.
    // Therefore, (value + mask) & ~mask = 10000 rounds 13 up to 16.
    return (T)(((size_t)value + mask) & ~mask);
  }

  // Aligns value on a given alignment boundary by rounding value down to the previous
  // multiple of alignment.
  template <typename T> __forceinline T alignDown(T value, size_t alignment) {
    return alignDownWithMask(value, alignment-1);
  }

  // Aligns value on a given alignment boundary by rounding value up to the next
  // multiple of alignment.
  template <typename T> __forceinline T alignUp(T value, size_t alignment) {
    return alignUpWithMask(value, alignment - 1);
  }

  template <typename T> inline size_t getHash(
    const T* stateDesc, 
    size_t count = 1,
    size_t hash = 2166136261U
  ) {
    static_assert((sizeof(T) & 3) == 0 && alignof(T) >= 4, "State object is not word-aligned");
    return hashRange((uint32_t*)stateDesc, (uint32_t*)(stateDesc + count), hash);
  }

  // Populates the buffer delimited by begin and end with accumulated 64-bit CRC32 hashes.
  // The 1st and/or last hash may be 32-bit CRC32 hashes if begin and/or end are not 
  // aligned on a 64-bit boundary.
  inline size_t hashRange(const uint32_t* const begin, const uint32_t* const end, size_t hash) {
    const uint64_t* u64Iter = (const uint64_t*)alignUp(begin, 8);
    const uint64_t* u64End = (const uint64_t*)alignDown(end, 8);
    
    if ((uint32_t*)u64Iter > begin) {
      // begin wasn't originally aligned on a 64-bit boundary, so it was
      // rounded up to the next 64-bit address and the result was stored in u64Iter.

      // Add a CRC32 value to hash; begin will point to the result.
      hash = _mm_crc32_u32((uint32_t)hash, *begin);
    }

    while (u64Iter < u64End) {
      // Add a 64-bit CRC32 value to the accumulated hash and store the results where
      // u64Iter currently points at.
      hash = _mm_crc32_u64((uint64_t)hash, *u64Iter++);
    }

    if ((uint32_t*)u64Iter < end) {
      // end may not have been aligned on a 64-bit boundary originally, so it
      // was rounded down to the previous 64-bit address and the result was
      // stored in u64End. Store a 32-bit hash at u64Iter.
      hash = __mm_crc32_u32((uint32_t)has, *(uint32_t*)u64Iter);
    }

    return hash;
  }
}

#ifdef ERROR
#undef ERROR
#endif
#ifdef ASSERT
#undef ASSERT
#endif
#ifdef HALT
#undef HALT
#endif

#define HALT( ... ) ERROR( __VA_ARGS__ ) __debugbreak();

#ifdef RELEASE

#define ASSERT( isTrue, ... ) (void)(isTrue)
#define WARN_ONCE_IF( isTrue, ... ) (void)(isTrue)
#define WARN_ONCE_IF_NOT( isTrue, ... ) (void)(isTrue)
#define ERROR( msg, ... )
#define DEBUGPRINT( msg, ... ) do {} while(0)
#define ASSERT_SUCCEEDED( hr, ... ) (void)(hr)

#else

#define STRINGIFY(x) #x
#define STRINGIFY_BUILTIN(x) STRINGIFY(x)
#define ASSERT( isFalse, ... ) \
        if (!(bool)(isFalse)) { \
            Utility::Print("\nAssertion failed in " STRINGIFY_BUILTIN(__FILE__) " @ " STRINGIFY_BUILTIN(__LINE__) "\n"); \
            Utility::PrintSubMessage("\'" #isFalse "\' is false"); \
            Utility::PrintSubMessage(__VA_ARGS__); \
            Utility::Print("\n"); \
            __debugbreak(); \
        }

#define ASSERT_SUCCEEDED( hr, ... ) \
        if (FAILED(hr)) { \
            Utility::Print("\nHRESULT failed in " STRINGIFY_BUILTIN(__FILE__) " @ " STRINGIFY_BUILTIN(__LINE__) "\n"); \
            Utility::PrintSubMessage("hr = 0x%08X", hr); \
            Utility::PrintSubMessage(__VA_ARGS__); \
            Utility::Print("\n"); \
            __debugbreak(); \
        }

#define WARN_ONCE_IF( isTrue, ... ) \
    { \
        static bool s_TriggeredWarning = false; \
        if ((bool)(isTrue) && !s_TriggeredWarning) { \
            s_TriggeredWarning = true; \
            Utility::Print("\nWarning issued in " STRINGIFY_BUILTIN(__FILE__) " @ " STRINGIFY_BUILTIN(__LINE__) "\n"); \
            Utility::PrintSubMessage("\'" #isTrue "\' is true"); \
            Utility::PrintSubMessage(__VA_ARGS__); \
            Utility::Print("\n"); \
        } \
    }

#define WARN_ONCE_IF_NOT( isTrue, ... ) WARN_ONCE_IF(!(isTrue), __VA_ARGS__)

#define ERROR( ... ) \
        Utility::Print("\nError reported in " STRINGIFY_BUILTIN(__FILE__) " @ " STRINGIFY_BUILTIN(__LINE__) "\n"); \
        Utility::PrintSubMessage(__VA_ARGS__); \
        Utility::Print("\n");

#define DEBUGPRINT( msg, ... ) \
    Utility::Printf( msg "\n", ##__VA_ARGS__ );

#endif

#define BreakIfFailed( hr ) if (FAILED(hr)) __debugbreak()