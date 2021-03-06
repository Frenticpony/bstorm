#pragma once

#include <d3dx9.h>

namespace bstorm
{
// 拡大、回転、移動の順番で掛けた行列を作る
D3DXMATRIX CreateScaleRotTransMatrix(float x, float y, float z, float rx, float ry, float rz, float sx, float sy, float sz);
}
