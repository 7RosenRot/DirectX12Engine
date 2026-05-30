// Принимаем текстуру из нашей Descriptor Table (t0)
Texture2D t_BaseColor : register(t0);

// Принимаем правила наложения (Static Sampler) (s0)
SamplerState s_Sampler : register(s0);

struct PS_INPUT {
    float4 Position : SV_POSITION;
    float2 UV       : TEXCOORD;
    float3 Normal   : NORMAL;
};

float4 PSMain(PS_INPUT input) : SV_TARGET {
    // 1. Нормализуем входящую нормаль
    float3 N = normalize(input.Normal);
    
    // 2. Направление света
    float3 L = normalize(float3(0.5f, 0.5f, -1.0f));
    
    // 3. Диффузное освещение
    float diff = saturate(dot(N, L));
    
    // 4. Фоновое освещение (Ambient)
    float ambient = 0.2f;
    float lightIntensity = diff + ambient;
    
    // 5. Читаем цвет из текстуры по переданным UV координатам
    float4 texColor = t_BaseColor.Sample(s_Sampler, input.UV);
    
    // 6. Умножаем цвет текстуры на свет (альфа-канал оставляем как есть)
    return float4(texColor.rgb * lightIntensity, texColor.a);
}