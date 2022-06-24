#include "Model.h"
#include "../Core/Graphics.h"

using Microsoft::WRL::ComPtr;
using namespace DirectX;

void Model::LoadModel(ID3D12Device* device, CommandQueue* commandQueue, std::string_view filePath)
{
	Assimp::Importer importer;
	
	const aiScene* scene = importer.ReadFile(filePath.data(), ASSIMP_PREPROCESS_FLAGS);
	
	
	if (!scene)
	{
		throw Error{NarrowToWideString(importer.GetErrorString()), L"App.cpp", 87, E_INVALIDARG};
	}

	size_t nextBaseVertexLocation = 0;
	size_t nextStartindexLocation = 0;
	ProcessNode(scene, scene->mRootNode, nextBaseVertexLocation, nextStartindexLocation);

	std::vector<Vertex> tempVertices;
	std::vector<uint16_t> tempIndices;
	
	// make one huge buffer for all meshes in model
	for (const auto mesh : m_meshes)
	{
		m_vertexBufferByteSize += sizeof(Vertex) * mesh.m_vertices.size();
		m_indexBufferByteSize += sizeof(uint16_t) * mesh.m_indices.size();
		tempVertices.insert(tempVertices.end(), mesh.m_vertices.begin(), mesh.m_vertices.end());
		tempIndices.insert(tempIndices.end(), mesh.m_indices.begin(), mesh.m_indices.end());
	}
	
	LoadBuffers(device, commandQueue, tempVertices, tempIndices);
}

void Model::ProcessNode(const aiScene* pScene, const aiNode* pNode, size_t& nextBaseVertexLocation, size_t& nextStartindexLocation)
{
	// static size_t nextBaseVertexLocation = 0;
	// static size_t nextStartindexLocation = 0;

	if (pNode)
	{
		for (size_t i = 0; i < pNode->mNumChildren; i++)
		{
			ProcessNode(pScene, pNode->mChildren[i], nextBaseVertexLocation, nextStartindexLocation);
		}
		for (size_t j = 0; j < pNode->mNumMeshes; j++)
		{
			// go into scene array at given index and process this node's meshes
			// mMeshes is an array of indexes into the scene
			const aiMesh* pMesh = pScene->mMeshes[pNode->mMeshes[j]];
			std::vector<Vertex> vertices(pMesh->mNumVertices);
			for (size_t k = 0; k < pMesh->mNumVertices; k++)
			{
				vertices[k].positon = XMFLOAT3(pMesh->mVertices[k].x, pMesh->mVertices[k].y, pMesh->mVertices[k].z);
				vertices[k].normal = XMFLOAT3(pMesh->mNormals[k].x, pMesh->mNormals[k].y, pMesh->mNormals[k].z);
			}
			std::vector<uint16_t> indices(pMesh->mNumFaces * 3);
			for (size_t i = 0; i < pMesh->mNumFaces; i++)
			{
				assert(pMesh->mFaces[i].mNumIndices == 3);
				indices.push_back(pMesh->mFaces[i].mIndices[0]);
				indices.push_back(pMesh->mFaces[i].mIndices[1]);
				indices.push_back(pMesh->mFaces[i].mIndices[2]);
			}
			// build up vertex and index locations as all meshes will be concatenated in on buffer
			auto tempVertexLocation = nextBaseVertexLocation;
			nextBaseVertexLocation += vertices.size();
			auto tempIndexLocation = nextStartindexLocation;
			nextStartindexLocation += indices.size();
			// move mesh
			m_meshes.emplace_back(Mesh{std::move(vertices), std::move(indices), tempVertexLocation, tempIndexLocation});
		}
	}
}

void Model::LoadBuffers(ID3D12Device* device, CommandQueue* commandQueue, const std::vector<Vertex>& vertices, const std::vector<uint16_t>& indices)
{
	auto commandList = commandQueue->GetCommandList();

	ComPtr<ID3D12Resource> intermediateVertexBuffer;
	CreateDefaultBuffer(device, commandList.Get(), m_vertexBuffer.ReleaseAndGetAddressOf(), 
		intermediateVertexBuffer.ReleaseAndGetAddressOf(), m_vertexBufferByteSize, vertices.data());

	m_vertexBufferView.BufferLocation = m_vertexBuffer->GetGPUVirtualAddress();
	m_vertexBufferView.SizeInBytes = unsigned(m_vertexBufferByteSize);
	m_vertexBufferView.StrideInBytes = sizeof(Vertex);

	ComPtr<ID3D12Resource> intermediateIndexBuffer;
	CreateDefaultBuffer(device, commandList.Get(), m_indexBuffer.ReleaseAndGetAddressOf(), 
		intermediateIndexBuffer.ReleaseAndGetAddressOf(), m_indexBufferByteSize, indices.data());

	m_indexBufferView.BufferLocation = m_indexBuffer->GetGPUVirtualAddress();
	m_indexBufferView.SizeInBytes = unsigned(m_indexBufferByteSize);
	m_indexBufferView.Format = DXGI_FORMAT_R16_UINT;

	// waiting for gpu, therefore ignore fence value
	commandQueue->ExecuteCommandList(commandList.Get());
	
	commandQueue->WaitForGPU();
}

void Model::LoadModelTinyObj(std::string_view filePath, std::vector<Vertex>& vertices)
{
	tinyobj::attrib_t attrib;
	std::vector<tinyobj::shape_t> shapes;
	std::vector<tinyobj::material_t> materials;
	std::string warn;
	std::string error;

	bool sucess = tinyobj::LoadObj(&attrib, &shapes, &materials, &warn, &error, filePath.data());

#ifdef _DEBUG
	if (!warn.empty())
	{
  		OutputDebugStringA(warn.c_str());
	}

	if (!error.empty())
	{
  		OutputDebugStringA(error.c_str());
	}
#endif

	if (!sucess)
	{
		throw Error{L"TinyObj failed to load model", L"App.cpp", 87, E_INVALIDARG};
	}

	// Loop over shapes
	for (size_t s = 0; s < shapes.size(); s++)
	{
		// Loop over faces(polygon)
	  	size_t index_offset = 0;
	  	for (size_t f = 0; f < shapes[s].mesh.num_face_vertices.size(); f++)
	  	{
	    	size_t fv = size_t(shapes[s].mesh.num_face_vertices[f]);

	    	// Loop over vertices in the face.
	    	for (size_t v = 0; v < fv; v++)
			{
	      		// access to vertex
				Vertex tempVertex{};
	      		tinyobj::index_t idx = shapes[s].mesh.indices[index_offset + v];

	      		tempVertex.positon.x = attrib.vertices[3*size_t(idx.vertex_index)+0];
	      		tempVertex.positon.y = attrib.vertices[3*size_t(idx.vertex_index)+1];
	      		tempVertex.positon.z = attrib.vertices[3*size_t(idx.vertex_index)+2];

	      		// Check if `normal_index` is zero or positive. negative = no normal data
	     	 	if (idx.normal_index >= 0)
				{
	     	   		tempVertex.normal.x = attrib.normals[3*size_t(idx.normal_index)+0];
	     	   		tempVertex.normal.y = attrib.normals[3*size_t(idx.normal_index)+1];
	        		tempVertex.normal.z = attrib.normals[3*size_t(idx.normal_index)+2];
	      		}

	      		// Check if `texcoord_index` is zero or positive. negative = no texcoord data
	     		if (idx.texcoord_index >= 0)
				{
	        		tinyobj::real_t tx = attrib.texcoords[2*size_t(idx.texcoord_index)+0];
	        		tinyobj::real_t ty = attrib.texcoords[2*size_t(idx.texcoord_index)+1];
	      		}
	      // Optional: vertex colors
	      // tinyobj::real_t red   = attrib.colors[3*size_t(idx.vertex_index)+0];
	      // tinyobj::real_t green = attrib.colors[3*size_t(idx.vertex_index)+1];
	      // tinyobj::real_t blue  = attrib.colors[3*size_t(idx.vertex_index)+2];
		  		vertices.push_back(tempVertex);
	    	}
	    	index_offset += fv;
	  	}
	}
}

void Model::Draw(ID3D12GraphicsCommandList* commandList)
{
	for (const auto mesh : m_meshes)
	{
		mesh.Draw(commandList);
	}
}