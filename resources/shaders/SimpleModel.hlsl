struct VSInput
{
    uint vertexID : SV_VertexID;
    float3 position : Attrib0;
    float3 normal : Attrib1;
    
    // Model matrix
    float3 modelColour : Attrib2;
    float4 mmRow0 : Attrib3;
    float4 mmRow1 : Attrib4;
    float4 mmRow2 : Attrib5;
    float4 mmRow3 : Attrib6;
};

struct PSInput
{
    float4 position : SV_Position;
    float3 normal : TEXCOORD0;
    float3 colour : TEXCOORD1;
};

struct PSOutput
{
    float4 colour : SV_Target;
};

// Constant for frame
cbuffer FrameConstants
{
    float4x4 ViewMatrix;
    float4x4 ProjectionMatrix;
    float3 CameraPosition;
};

void VSMain(in VSInput input, out PSInput output)
{
    // This might be changed to just use the normal translation, rotation, scale and calculate matrix here. 
    float4x4 modelMatrix = MatrixFromRows(input.mmRow0, input.mmRow1, input.mmRow2, input.mmRow3);
    
    float4 positionWorld = mul(float4(input.position, 1.0f), modelMatrix);
    
    output.position = mul(positionWorld, mul(ViewMatrix, ProjectionMatrix));
    //output.position = mul(float4(input.position, 1.0f), mul(ViewMatrix, ProjectionMatrix));
    output.normal = input.normal;
    output.colour = input.modelColour;
}

void PSMain(in PSInput input, out PSOutput output)
{
    //output.colour = float4(input.colour, 1.0f);
    output.colour = float4(abs(input.normal), 1.0f);
}
