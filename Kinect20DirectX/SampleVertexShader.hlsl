// Per-vertex data used as input to the vertex shader.
cbuffer ConstantBuffer : register(b0)
{
    matrix World;
    matrix View;
    matrix Projection;
}

struct VertexShaderInput
{
	float3 pos : POSITION;
	float3 color : COLOR0;
    float width : PSIZE0; 
};


struct GeometryShaderInput
{
	float4 pos : SV_POSITION;
	float3 color : COLOR0;
    float width : PSIZE0;
};

// Simple shader to do vertex processing on the GPU.
GeometryShaderInput main(VertexShaderInput input)
{
    GeometryShaderInput output;
	output.pos = float4(input.pos, 1.0f);

	// Pass the color through without modification.
	output.color = input.color;

    output.width = input.width;

	return output;
}


//--------------------------------------------------------------------------------------
// Vertex Shader for Axis
//--------------------------------------------------------------------------------------
GeometryShaderInput AxisVertexShader(VertexShaderInput input)
{
    GeometryShaderInput output = (GeometryShaderInput)0;
    output.pos = float4(input.pos, 1.0f);
    output.pos = mul(output.pos, World);
    output.pos = mul(output.pos, View);
    output.pos = mul(output.pos, Projection);
    output.color = input.color;
    output.width = input.width;

    return output;
}