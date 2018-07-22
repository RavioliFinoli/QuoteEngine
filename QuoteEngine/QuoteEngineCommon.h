#pragma once
#include <DirectXMath.h>
#include "QEModel.h"
#define ASSERT_SOK(x) assert(x == S_OK)
#define MOVEMENT_SPEED_MODIFIER 1.0f

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

	struct CB_PerModel_WVP_W
	{
		DirectX::XMFLOAT4X4 _WVP;
		DirectX::XMFLOAT4X4 _W;
	};
	
	struct CB_PerFrame_Cam 
	{
		DirectX::XMFLOAT3 _CamPosition;
	};

	struct CB_PerModel_Material
	{
		QEModel::QEMaterial _MaterialAttributes;
	};
}