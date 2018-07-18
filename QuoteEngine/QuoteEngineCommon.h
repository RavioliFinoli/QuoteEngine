#pragma once
#include <DirectXMath.h>

#define ASSERT_SOK(x) assert(x == S_OK)

namespace QuoteEngine
{
	enum class SHADER_TYPE
	{
		VERTEX_SHADER,
		PIXEL_SHADER,
		GEOMETRY_SHADER,
		COMPUTE_SHADER
	};

	struct CB_PerModel
	{
		DirectX::XMFLOAT4X4 _WVP;
	};
}