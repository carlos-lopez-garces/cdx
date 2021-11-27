#pragma once

#include <DirectXMath.h>

using namespace DirectX;

class Color {
public:
  Color() : mValue(g_XMOne) {}

  float R() const {
    return XMVectorGetX(mValue);
  }

  float G() const {
    return XMVectorGetY(mValue);
  }

  float B() const {
    return XMVectorGetZ(mValue);
  }

  float A() const {
    return XMVectorGetW(mValue);
  }

private:
  XMVECTORF32 mValue;
};