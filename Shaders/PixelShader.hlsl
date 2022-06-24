struct PixelIn
{
	float3 normal : NORMAL;
	float3 fragPositon : POSITIONT;
};

static const float ambientStrength = 0.01f;

static const float specularStrength = 0.5f;

static const float3 lightPosition = {1.2f, 1.0f, -7.5f};

static const float3 viewPositon = {0.0f, 0.0f, -2.5f};

static const float3 lightColour = {1.0f, 1.0f, 1.0f};

static const float3 objectColour = {0.67f, 0.66f, 0.68f};

float4 main(PixelIn pin) : SV_Target
{
	float3 normal = normalize(pin.normal);

	float3 directionToLight = normalize(lightPosition - pin.fragPositon);

	float3 viewDirection = normalize(viewPositon - pin.fragPositon);
	
	float3 ambient = ambientStrength * lightColour;

	float diff = max(dot(normal, directionToLight), 0.0f);

	float3 diffuse = diff * lightColour;

	float3 reflectedLight = reflect(-directionToLight, normal);

	float spec = pow(max(dot(reflectedLight, viewDirection), 0.0f), 512);

	float3 specular = spec * specularStrength * lightColour;

	return float4(objectColour * (diffuse + ambient + specular), 1.0f);
}