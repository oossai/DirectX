#include "Mesh.h"

Mesh::Mesh(std::vector<Vertex>&& vertices, std::vector<uint16_t>&& indices, size_t baseVertexLocation, size_t startIndexLocation)
	: m_vertices{vertices}, m_indices{indices}, m_baseVertexLocation{baseVertexLocation}, m_startIndexLocation{startIndexLocation}
{}

void Mesh::Draw(ID3D12GraphicsCommandList* commandList) const 
{ 
	commandList->DrawIndexedInstanced((unsigned)m_indices.size(), 
		1u, (unsigned)m_startIndexLocation, (unsigned)m_baseVertexLocation, 0u); 
}