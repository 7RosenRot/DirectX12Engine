cbuffer SceneData : register(b0) {
    float4x4 modelMatrix;
};

struct VS_INPUT {
    float3 Position : POSITION;
    float2 UV       : TEXCOORD; // Порядок не строгий, но лучше выравнивать как в C++
    float3 Normal   : NORMAL;
};

struct VS_OUTPUT {
    float4 Position : SV_POSITION;
    float2 UV       : TEXCOORD;
    float3 Normal   : NORMAL;
};

VS_OUTPUT VSMain(VS_INPUT input) {
    VS_OUTPUT output;
    
    // Считаем позицию на экране
    output.Position = mul(float4(input.Position, 1.0f), modelMatrix);
    
    // Вращаем нормали (приводим матрицу 4x4 к 3x3, чтобы отсечь перемещение)
    output.Normal = mul(input.Normal, (float3x3)modelMatrix);
    
    // Просто передаем UV координаты дальше в пиксельный шейдер
    output.UV = input.UV;
    
    return output;
}