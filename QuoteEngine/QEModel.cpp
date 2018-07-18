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
	m_WorldMatrix = DirectX::XMMatrixIdentity();
	create();
}

QEModel::QEModel(std::string file) noexcept
{
	using std::string;
	using QuoteEngine::Vertex_pos3;
	using QuoteEngine::Vertex_uv2;
	using QuoteEngine::Vertex_pos3nor3uv2;

	std::ifstream OBJFile;
	//Open file
	string path = string(string("Resources/") + file + ".obj");
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
				//DWORD num1, num2, num3;
				//std::string garbage;
				//OBJFile >> 
				//	num1 >>
				//	garbage >> 
				//	num2 >>
				//	garbage >> 
				//	num3 >>
				//	garbage;

				//indicies.push_back(num1 - 1);
				//indicies.push_back(num2 - 1);
				//indicies.push_back(num3 - 1);
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
}


QEModel::~QEModel()
{
}

HRESULT QEModel::create()
{
	struct TriangleVertex
	{
		float x, y, z;
		float r, g, b;
	};

	// Array of Structs (AoS)
	TriangleVertex triangleVertices[3] =
	{
		0.0f, 0.5f, 0.0f,	//v0 pos
		1.0f, 0.0f, 0.0f,	//v0 color

		0.5f, -0.5f, 0.0f,	//v1
		0.0f, 1.0f, 0.0f,	//v1 color

		-0.5f, -0.5f, 0.0f, //v2
		0.0f, 0.0f, 1.0f	//v2 color
	};

	m_SizeInBytes = sizeof(TriangleVertex) * 3;
	m_VertexCount = 3;
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
