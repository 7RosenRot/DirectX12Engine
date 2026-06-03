Texture2D t_BaseColor : register(t0);
SamplerState s_Sampler : register(s0);

struct PS_INPUT {
  float4 Position      : SV_POSITION;
  
  float3 WorldPosition : POSITION;
  float2 UV            : TEXCOORD;
  float3 Normal        : NORMAL;
};

float4 PSMain(PS_INPUT Input) : SV_TARGET {
  float3 N = normalize(Input.Normal);               // Normal from face _↑_                  *
  float3 L = normalize(float3(1.0f, 1.0f, -1.0f));  // Normal from pixel to light origin _↑↗_
  float  diffuse  = max(dot(N, L), 0.0f);
  
  float4 albedo = t_BaseColor.Sample(s_Sampler, Input.UV); // Base color (no light/shadows)


  float3 finalColor = albedo.rgb * (diffuse + 0.2f);
  
  return float4(finalColor, 1.0f);
}