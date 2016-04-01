// A constant buffer that stores the three basic column-major matrices for composing geometry.
cbuffer ProjectionConstantBuffer : register(b0)
{
    matrix projection;
};

cbuffer ViewConstantBuffer : register(b1)
{
    matrix view;
};

cbuffer ModelConstantBuffer : register(b2)
{
    matrix model;
};



// Per-pixel color data passed through the pixel shader.
struct PixelShaderInput
{
    float4 pos : SV_POSITION;
    float3 color : COLOR0;
};

struct GeometryShaderInput
{
    float4 pos : SV_POSITION;
    float3 color : COLOR0;
    float width : PSIZE0;
};

[maxvertexcount(24)]
void main(
	point GeometryShaderInput gsIn[1],
	inout TriangleStream< PixelShaderInput > output
)
{
	// Create a cube around the point
    // It consists of vertices at the near top-left, near top right, ..., far top-left, far top right ....
    PixelShaderInput ntl, ntr, nbr, nbl, ftl, ftr, fbr, fbl;
    
    ntl.color = 
        ntr.color = nbr.color = 
        nbl.color = ftl.color =
        ftr.color = fbr.color = 
        fbl.color = gsIn[0].color;


    float width2 = gsIn[0].width / 2;

    // Near top left vertex
    ntl.pos = float4(
        (gsIn[0].pos[0]) - (width2),
        (gsIn[0].pos[1]) + (width2),
        (gsIn[0].pos[2]) + (width2),
        1.0f
        );

    // Near top right vertex
    ntr.pos = float4(
        (gsIn[0].pos[0]) + (width2),
        (gsIn[0].pos[1]) + (width2),
        (gsIn[0].pos[2]) + (width2),
        1.0f
        );
    
    // Near bottom left vertex
    nbl.pos = float4(
        (gsIn[0].pos[0]) - (width2),
        (gsIn[0].pos[1]) - (width2),
        (gsIn[0].pos[2]) + (width2),
        1.0f
        );

    // Near bottom right vertex
    nbr.pos = float4(
        (gsIn[0].pos[0]) + (width2),
        (gsIn[0].pos[1]) - (width2),
        (gsIn[0].pos[2]) + (width2),
        1.0f
        );


    // Far top left vertex
    ftl.pos = float4(
        (gsIn[0].pos[0]) - (width2),
        (gsIn[0].pos[1]) + (width2),
        (gsIn[0].pos[2]) - (width2),
        1.0f
        );

    // Far top right vertex
    ftr.pos = float4(
        (gsIn[0].pos[0]) + (width2),
        (gsIn[0].pos[1]) + (width2),
        (gsIn[0].pos[2]) - (width2),
        1.0f
        );

    // Far bottom left vertex
    fbl.pos = float4(
        (gsIn[0].pos[0]) - (width2),
        (gsIn[0].pos[1]) - (width2),
        (gsIn[0].pos[2]) - (width2),
        1.0f
        );

    // Far bottom right vertex
    fbr.pos = float4(
        (gsIn[0].pos[0]) + (width2),
        (gsIn[0].pos[1]) - (width2),
        (gsIn[0].pos[2]) - (width2),
        1.0f
        );

    ntl.pos = mul(ntl.pos, model);
    ntl.pos = mul(ntl.pos, view);
    ntl.pos = mul(ntl.pos, projection);

    ntr.pos = mul(ntr.pos, model);
    ntr.pos = mul(ntr.pos, view);
    ntr.pos = mul(ntr.pos, projection);

    nbl.pos = mul(nbl.pos, model);
    nbl.pos = mul(nbl.pos, view);
    nbl.pos = mul(nbl.pos, projection);

    nbr.pos = mul(nbr.pos, model);
    nbr.pos = mul(nbr.pos, view);
    nbr.pos = mul(nbr.pos, projection);

    ftl.pos = mul(ftl.pos, model);
    ftl.pos = mul(ftl.pos, view);
    ftl.pos = mul(ftl.pos, projection);
                 
    ftr.pos = mul(ftr.pos, model);
    ftr.pos = mul(ftr.pos, view);
    ftr.pos = mul(ftr.pos, projection);
                
    fbl.pos = mul(fbl.pos, model);
    fbl.pos = mul(fbl.pos, view);
    fbl.pos = mul(fbl.pos, projection);
                 
    fbr.pos = mul(fbr.pos, model);
    fbr.pos = mul(fbr.pos, view);
    fbr.pos = mul(fbr.pos, projection);

    // Now we have created all 8 vertices of the cube, let's create the faces
    // Each face is 2 triangles represented by the strip abcd, creating triangles ABC and BCD
    // a...b    a...b       b
    // ..... ==>...       ...
    // c...d    c       c...d

    // Front face
    output.Append(ntl);
    output.Append(ntr);
    output.Append(nbl);
    output.Append(nbr);
    output.RestartStrip();

    // Back face
    output.Append(ftr);
    output.Append(ftl);
    output.Append(fbr);
    output.Append(fbl);
    output.RestartStrip();

    // Left face
    output.Append(ftl);
    output.Append(ntl);
    output.Append(fbl);
    output.Append(nbl);
    output.RestartStrip();

    // Right face
    output.Append(ntr);
    output.Append(ftr);
    output.Append(nbr);
    output.Append(fbr);
    output.RestartStrip();

    // Bottom face
    output.Append(fbl);
    output.Append(nbl);
    output.Append(fbr);
    output.Append(nbr);
    output.RestartStrip();

     //Top face
    output.Append(ftr);
    output.Append(ntr);
    output.Append(ftl);
    output.Append(ntl);
    output.RestartStrip();
}