struct ObjectConstant
{
	float4x4 mvp;
};

struct ObjectConstant2
{
	float4x4 model;
};

ConstantBuffer<ObjectConstant> cBuffer : register (b0);
ConstantBuffer<ObjectConstant2> cBuffer2 : register (b1);

struct VertexIn
{
	float3 position : POSITION;
	float3 normal : NORMAL;
};

struct VertexOut
{
	float3 normal : NORMAL;
	float3 fragPositon : POSITIONT;
	float4 positon : SV_Position;
};

VertexOut main(VertexIn vin)
{
	VertexOut vout;
	vout.positon = mul(cBuffer.mvp, float4(vin.position, 1.0));
	vout.normal = mul((float3x3)(cBuffer2.model), vin.normal);
	vout.fragPositon = mul(cBuffer2.model, float4(vin.position, 1.0)).xyz;
	return vout;
}