struct PS_INPUT {
    float4 Position : SV_POSITION;
    float2 UV       : TEXCOORD;
    float3 Normal   : NORMAL;
};

float4 PSMain(PS_INPUT input) : SV_TARGET {
    // 1. Нормализуем входящую нормаль (после интерполяции она может "уплыть")
    float3 N = normalize(input.Normal);
    
    // 2. Задаем направление света (пусть светит чуть сверху и спереди)
    float3 L = normalize(float3(0.5f, 0.5f, -1.0f));
    
    // 3. Считаем коэффициент освещенности (скалярное произведение)
    // saturate обрезает значения меньше 0
    float diff = saturate(dot(N, L));
    
    // 4. Добавляем немного фонового света (Ambient), чтобы тени не были абсолютно черными
    float ambient = 0.2f;
    float lightIntensity = diff + ambient;
    
    // 5. Итоговый цвет (серый металлик для Сюзанны)
    float3 baseColor = float3(0.7f, 0.7f, 0.8f); 
    
    return float4(baseColor * lightIntensity, 1.0f);
}