#include "QEModel.h"
#include "QERenderingModule.h"
#include <fstream>

using QuoteEngine::QERenderingModule;

inline void split(const std::string& s, char c,
	std::vector<std::string>& v) {
	std::string::size_type i = 0;
	std::string::size_type j = s.find(c);

	while (j != std::string::npos) {
		v.push_back(s.substr(i, j - i));
		i = ++j;
		j = s.find(c, j);
		if (j == std::string::npos)
			v.push_back(s.substr(i, s.length()));
	}
}

QEModel::QEModel() 
{
	m_VertexBuffer.Reset(); //clean ComPtr
	m_WorldMatrices.push_back(DirectX::XMMatrixIdentity());
	create();
}

QEModel::QEModel(std::string file) noexcept
{
	//set floats to 0
	memset(m_Translation, 0, sizeof(float) * 9);

	m_WorldMatrices.push_back(DirectX::XMMatrixIdentity());
	loadFromFile(file);
}


QEModel::QEModel(std::string file, std::string shaderProgram) noexcept
{
	//set trans/rot to 0, scale to 1
	memset(m_Translation, 0, sizeof(float) * 6);
	
	m_Scale[0] = 1.0;
	m_Scale[1] = 1.0;
	m_Scale[2] = 1.0;

	m_WorldMatrices.push_back(DirectX::XMMatrixIdentity());
	loadFromFile(file);
	setAssociatedShaderProgram(shaderProgram);
}

QEModel::~QEModel()
{
}

std::string QEModel::getAssociatedShaderProgram()
{
	return m_AssociatedShaderProgramName;
}

QEModel::QEMaterial QEModel::getMaterial()
{
	return m_Material;
}

void QEModel::Update()
{
	for (auto& instance : m_WorldMatrices)
	{
		auto translation = DirectX::XMMatrixTranslation(m_Translation[0], m_Translation[1], m_Translation[2]);
		auto rotation = DirectX::XMMatrixRotationRollPitchYaw(m_Rotation[0], m_Rotation[1], m_Rotation[2]);
		auto scale = DirectX::XMMatrixScaling(m_Scale[0], m_Scale[1], m_Scale[2]);
		instance = DirectX::XMMatrixMultiply(translation, DirectX::XMMatrixMultiply(rotation, scale));
	}
}

void QEModel::setAssociatedShaderProgram(std::string program)
{
	m_AssociatedShaderProgramName = program;
}

void QEModel::setRawTransformations(float* t, float* r, float* s)
{
	m_Translation[0] = t[0];
	m_Translation[1] = t[1];
	m_Translation[2] = t[2];

	m_Rotation[0] = r[0];
	m_Rotation[1] = r[1];
	m_Rotation[2] = r[2];

	m_Scale[0] = s[0];
	m_Scale[1] = s[1];
	m_Scale[2] = s[2];
}

std::vector<DirectX::XMMATRIX>& QEModel::getWorldMatrices()
{
	return m_WorldMatrices;
}

void QEModel::setWorldMatrix(DirectX::XMMATRIX worldMatrix, UINT instance)
{
	if (m_WorldMatrices.size() > instance)
		m_WorldMatrices.at(instance) = worldMatrix;
	else if (instance > m_WorldMatrices.size() + 1)
		return;
	else
		m_WorldMatrices.push_back(worldMatrix);
}

void QEModel::move(float x, float y, float z)
{
	m_Translation[0] += x;
	m_Translation[1] += y;
	m_Translation[2] += z;
	//for (auto& instance : m_WorldMatrices)
	//	instance = DirectX::XMMatrixMultiply(instance, DirectX::XMMatrixTranslation(x, y, z));
}

void QEModel::rotate(float x, float y, float z)
{
	m_Rotation[0] += x;
	m_Rotation[1] += y;
	m_Rotation[2] += z;
	//for (auto& instance : m_WorldMatrices)
	//	instance = DirectX::XMMatrixMultiply(instance, DirectX::XMMatrixRotationRollPitchYaw(x, y, z));
}

void QEModel::bindTexture()
{
	if (m_Texture)
		m_Texture->bind();
}

bool QEModel::hasAssociatedShaderProgram()
{
	return m_AssociatedShaderProgramName != "none";
}

HRESULT QEModel::create()
{
	struct TriangleVertex
	{
		float x, y, z;
;	};

	// Array of Structs (AoS)
	TriangleVertex triangleVertices[6] =
	{
		-2.0f, 2.0f, 0.0f,	//v0 pos
		2.0f, 2.0f, 0.0f,	//v0 color

		2.0f, -2.0f, 0.0f,	//v1
		-2.0f, 2.0f, 0.0f,	//v1 color

		2.0f, -2.0f, 0.0f, //v2
		-2.0f, -2.0f, 0.0f,	//v2 color
	};

	m_SizeInBytes = sizeof(TriangleVertex) * 6;
	m_VertexCount = 6;
	m_StrideInBytes = sizeof(TriangleVertex);

	// Describe the Vertex Buffer
	D3D11_BUFFER_DESC bufferDesc;
	memset(&bufferDesc, 0, sizeof(bufferDesc));
	// what type of buffer will this be?
	bufferDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	// what type of usage (press F1, read the docs)
	bufferDesc.Usage = D3D11_USAGE_DEFAULT;
	// how big in bytes each element in the buffer is.
	bufferDesc.ByteWidth = sizeof(triangleVertices);

	// this struct is created just to set a pointer to the
	// data containing the vertices.
	D3D11_SUBRESOURCE_DATA data;
	data.pSysMem = triangleVertices;

	// create a Vertex Buffer
	return QERenderingModule::gDevice->CreateBuffer(&bufferDesc, &data, m_VertexBuffer.ReleaseAndGetAddressOf());
}

void QEModel::loadFromFile(std::string file)
{
	using std::string;
	using QuoteEngine::Vertex_pos3;
	using QuoteEngine::Vertex_uv2;
	using QuoteEngine::Vertex_pos3nor3uv2;

	std::ifstream OBJFile;
	//Open file
	string path = string(file + ".obj");
	OBJFile.open(path);

	assert(OBJFile.is_open() && "obj file not open");

	std::vector<Vertex_pos3> positions;
	std::vector<Vertex_pos3> normals;
	std::vector<Vertex_uv2> uvs;
	std::vector<Vertex_pos3nor3uv2> verts;
	std::vector<UINT> indicies;
	std::string mtllib;
	bool useTexture = false;
	//Parse vertex, UV and Normal information from the file
	if (OBJFile.is_open())
	{
		std::string input;
		while (!OBJFile.eof())
		{
			OBJFile >> input;
			if (input == "v")
			{
				Vertex_pos3 v;
				OBJFile >> v.x >> v.y >> v.z;
				positions.push_back(v);
			}
			else if (input == "vt")
			{
				Vertex_uv2 v;
				OBJFile >> v.u >> v.v;
				uvs.push_back(v);
			}
			else if (input == "vn")
			{
				Vertex_pos3 v;
				OBJFile >> v.x >> v.y >> v.z;
				normals.push_back(v);
			}
			else if (input == "f")
			{
				std::string line;

				//get the three verts as strings ("X/X/X")
				std::vector<std::string> vStrings;
				std::getline(OBJFile, line);
				split(line, ' ', vStrings);

				//for each vertex
				for (int i = 1; i < vStrings.size(); i++) //v[0] is empty, so we start at [1]
				{
					std::vector<std::string> vIndicies;
					vIndicies.clear();

					Vertex_pos3nor3uv2 vertex;
					ZeroMemory(&vertex, sizeof(vertex));

					//split vertex. We now have index of v, vt and vn as strings.
					split(vStrings[i], '/', vIndicies);

					//positions
					vertex.posX = positions[std::stoi(vIndicies[0]) - 1].x;
					vertex.posY = positions[std::stoi(vIndicies[0]) - 1].y;
					vertex.posZ = positions[std::stoi(vIndicies[0]) - 1].z;

					//UVS
					vertex.u = uvs[std::stoi(vIndicies[1]) - 1].u;
					vertex.v = uvs[std::stoi(vIndicies[1]) - 1].v;

					//normals
					vertex.norX = normals[std::stoi(vIndicies[2]) - 1].x;
					vertex.norY = normals[std::stoi(vIndicies[2]) - 1].y;
					vertex.norZ = normals[std::stoi(vIndicies[2]) - 1].z;

					verts.push_back(vertex);
				}
			}

			else if (input == "mtllib")
			{
				OBJFile >> mtllib;
			}

		}
	}

	//Create vertex buffer
	{
		// Describe the Vertex Buffer
		D3D11_BUFFER_DESC bufferDesc;
		memset(&bufferDesc, 0, sizeof(bufferDesc));
		// what type of buffer will this be?
		bufferDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
		// what type of usage (press F1, read the docs)
		bufferDesc.Usage = D3D11_USAGE_DEFAULT;
		// how big in bytes each element in the buffer is.
		bufferDesc.ByteWidth = verts.size() * sizeof(Vertex_pos3nor3uv2);

		// this struct is created just to set a pointer to the
		// data containing the vertices.
		D3D11_SUBRESOURCE_DATA data;
		data.pSysMem = verts.data();

		m_VertexCount = verts.size();
		m_StrideInBytes = sizeof(Vertex_pos3nor3uv2);


		// create a Vertex Buffer
		HRESULT hr = QuoteEngine::QERenderingModule::gDevice->CreateBuffer(&bufferDesc, &data, m_VertexBuffer.ReleaseAndGetAddressOf());
		assert(hr == S_OK);
	}

	if (mtllib.size() > 0)
	{
		std::ifstream MTLFile;
		//open
		std::string mtlfile = std::string(mtllib);
		MTLFile.open(mtlfile);

		assert(MTLFile.is_open() && "mtl file not open");

		std::string input;
		while (!MTLFile.eof())
		{
			MTLFile >> input;

			//-----------------
			// Diffuse Texture
			//-----------------
			if (input == "map_Kd")
			{
				useTexture = true;

				MTLFile >> input;
				string narrow_string(input);
				std::wstring wide_string = std::wstring(narrow_string.begin(), narrow_string.end());
				//this->LoadTexture(wide_string.c_str());

				//Create texture
				m_Texture = std::make_unique<QuoteEngine::QETexture>(wide_string.c_str());
			}
			else if (input == "Ks")
			{
				MTLFile >> m_Material.KsR >> m_Material.KsG >> m_Material.KsB;
			}
			else if (input == "Ka")
			{
				MTLFile >> m_Material.KaR >> m_Material.KaG >> m_Material.KaB;
			}
			else if (input == "Kd")
			{
				MTLFile >> m_Material.KdR >> m_Material.KdG >> m_Material.KdB;
			}
			else if (input == "Ns")
			{
				MTLFile >> m_Material.Ns;
			}
		}

		(useTexture) ?
			m_Material.UseTexture = 1.0f
			: m_Material.UseTexture = -1.0f;

		MTLFile.close();
	}

}
