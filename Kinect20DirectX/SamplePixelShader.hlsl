// Per-pixel color data passed through the pixel shader.
struct GeometryShaderInput
{
    float4 pos : SV_POSITION;
    float3 color : COLOR0;
    float width : PSIZE0;
};


// A pass-through function for the (interpolated) color data.
float4 main(GeometryShaderInput input) : SV_TARGET
{
	return float4(input.color, 1.0f);
}
