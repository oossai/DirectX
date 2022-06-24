#ifndef __MODEL_H__
#define __MODEL_H__

#include "../Headers.h"
#include "../Error.h"
#include "../Utility/Utility.h"
#include "Mesh.h"
#include "../Macros.h"

class CommandQueue;

class Model
{
private:
	std::vector<Mesh> m_meshes;
public:
	Model(){};
	~Model(){};
	void Draw(ID3D12GraphicsCommandList* commandList);
	void LoadModel(ID3D12Device* device, CommandQueue* commandQueue, std::string_view filePath);
	D3D12_VERTEX_BUFFER_VIEW GetVertexBufferView(){ return m_vertexBufferView; }
	D3D12_INDEX_BUFFER_VIEW GetIndexBufferView(){ return m_indexBufferView; }
private:
	NO_COPY_OR_MOVE(Model);
	void ProcessNode(const aiScene* pScene, const aiNode* pNode, size_t& nextBaseVertexLocation, size_t& nextStartindexLocation);
	void LoadModelTinyObj(std::string_view filePath, std::vector<Vertex>& vertices);
	void LoadBuffers(ID3D12Device* device, CommandQueue* commandQueue, const std::vector<Vertex>& vertices, const std::vector<uint16_t>& indices);
private:
	Microsoft::WRL::ComPtr<ID3D12Resource> m_vertexBuffer;
	Microsoft::WRL::ComPtr<ID3D12Resource> m_indexBuffer;
	D3D12_VERTEX_BUFFER_VIEW m_vertexBufferView;
	D3D12_INDEX_BUFFER_VIEW m_indexBufferView;
	size_t m_vertexBufferByteSize = 0;
	size_t m_indexBufferByteSize = 0;
};

#endif