#include "QEScene.h"
#include <DirectXMath.h>

QEScene::QEScene(UINT id) : m_SceneID(id)
{
}

void QEScene::addModel(std::string name, std::string shaderProgram, std::vector<DirectX::XMMATRIX> matrices)
{
	m_Models.push_back(std::make_shared<QEModel>(name, shaderProgram));
	for (int i = 0; i < matrices.size(); i++)
	{
		m_Models.back()->setWorldMatrix(matrices[i], i);
	}
	m_ModelNames.push_back(name);
}

std::vector<std::shared_ptr<QEModel>>& QEScene::getModels()
{
	return m_Models;
}

std::vector<std::string> QEScene::getModelNames()
{
	return m_ModelNames;
}

QEScene::~QEScene()
{
}
