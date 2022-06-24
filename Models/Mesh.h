#ifndef __MESH_H__
#define __MESH_H__

#include "../Headers.h"

struct Vertex
{
	DirectX::XMFLOAT3 positon;
	DirectX::XMFLOAT3 normal;

	bool operator==(const Vertex& rhs)
	{
		DirectX::XMVECTOR lhsPositon = DirectX::XMLoadFloat3(&positon);
		DirectX::XMVECTOR lhsNormal = DirectX::XMLoadFloat3(&normal);
		DirectX::XMVECTOR rhsPositon = DirectX::XMLoadFloat3(&rhs.positon);
		DirectX::XMVECTOR rhsNormal = DirectX::XMLoadFloat3(&rhs.normal);
		return DirectX::XMVector3Equal(lhsPositon, rhsPositon) &&
			DirectX::XMVector3Equal(lhsNormal, rhsNormal);	
	}
};

class Mesh
{
friend class Model;	
private:
	std::vector<Vertex> m_vertices;
	std::vector<uint16_t> m_indices;
	size_t m_startIndexLocation = 0;
	size_t m_baseVertexLocation = 0;
public:
	Mesh(std::vector<Vertex>&& vertices, std::vector<uint16_t>&& indices, size_t baseVertexLocation, size_t startIndexLocation);
	void Draw(ID3D12GraphicsCommandList* commandList) const;
};

#endif