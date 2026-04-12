cbuffer SceneData : register(b0) {
    float4x4 modelMatrix;
};

struct VS_INPUT {
    float3 Position : POSITION;
    float3 Normal   : NORMAL;
    float2 UV       : TEXCOORD;
};

struct VS_OUTPUT {
    float4 Position : SV_POSITION;
    float2 UV       : TEXCOORD;
    float3 Normal   : NORMAL;
    
    float4 Color    : COLOR;
};

VS_OUTPUT VSMain(VS_INPUT input) {
    VS_OUTPUT output;
    
    output.Position = mul(float4(input.Position, 1.0f), modelMatrix);
    
    output.Normal = mul(input.Normal, (float3x3)modelMatrix);
    
    output.UV = input.UV;

    output.Color = float4(1.0f, 1.0f, 1.0f, 1.0f);
    
    return output;
}