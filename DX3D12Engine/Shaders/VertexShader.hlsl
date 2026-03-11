struct VSInput {
  float4 position : SV_POSITION;
  float4 color : COLOR;
};

VSInput VSMain(float4 color : COLOR, float4 position : POSITION) {
  VSInput output;

  output.position = position;
  output.color = color;

  return output;
}