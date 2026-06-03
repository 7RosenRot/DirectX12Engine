cbuffer SceneData : register(b0) {
  float4x4 ModelMatrix;
  float4x4 ViewProjectionMatrix;
};

struct VS_INPUT {
  float3 Position : POSITION;
  float2 UV       : TEXCOORD;
  float3 Normal   : NORMAL;
};

struct VS_OUTPUT {
  float4 Position      : SV_POSITION;
  
  float3 WorldPosition : POSITION;
  float2 UV            : TEXCOORD;
  float3 Normal        : NORMAL;
};

VS_OUTPUT VSMain(VS_INPUT Input) {
  VS_OUTPUT Output;
  
  Output.Position = mul(float4(Input.Position, 1.0f), ModelMatrix);

  Output.WorldPosition = mul(float4(Input.Position, 1.0f), ModelMatrix).xyz;
  Output.UV = Input.UV;
  Output.Normal = mul(Input.Normal, (float3x3)ModelMatrix);
  
  return Output;
}