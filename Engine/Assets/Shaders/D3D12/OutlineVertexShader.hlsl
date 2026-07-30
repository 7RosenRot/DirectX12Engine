struct VertexInput {
  float3 Position : POSITION;
  float2 TexCoord : TEXCOORD;
  float3 Normal : NORMAL;
};

struct VertexOutput {
  float4 Position : SV_POSITION;
};

cbuffer Constants : register(b0) {
  matrix ViewProjection;
};

VertexOutput VSMain(VertexInput Input) {
  VertexOutput Output;
  float outlineThickness = 0.015f; // Outline width
  float3 pos = Input.Position + Input.Normal * outlineThickness;
  Output.Position = mul(float4(pos, 1.0f), ViewProjection);
  return Output;
}
