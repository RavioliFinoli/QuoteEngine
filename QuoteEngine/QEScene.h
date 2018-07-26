#pragma once
#include <wtypes.h>
#include "QEModel.h"
#include <vector>
#include <fstream>

class QEScene
{
public:
	QEScene(UINT id);
	void addModel(std::string name, std::string shaderProgram, std::vector<DirectX::XMMATRIX> matrices);
	std::vector<std::shared_ptr<QEModel>>& getModels();
	std::vector<std::string> getModelNames();
	~QEScene();
private:
	UINT m_SceneID;
	std::vector<std::shared_ptr<QEModel>> m_Models;
	std::vector<std::string> m_ModelNames;
};

