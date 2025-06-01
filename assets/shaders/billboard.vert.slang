struct ViewParams {
  float4x4 pv;
  float3 viewPos;
};

struct VSInput {
  uint idx : SV_VertexID;
};

struct VSOutput {
  float4x4 pv;
  float4 position : SV_Position;
};

static const float2[] quadXYVertices = {
  float2(-1.0f, +1.0f),  // bottom left
  float2(-1.0f, -1.0f),  // top left
  float2(+1.0f, -1.0f),  // top right
  float2(+1.0f, -1.0f),  // top right
  float2(+1.0f, +1.0f),  // bottom right
  float2(-1.0f, +1.0f),  // bottom left
};

layout(set = 1, binding = 0) ConstantBuffer<ViewParams> viewParams;

[shader("vertex")]
VSOutput vertexMain(VSInput input) {
  VSOutput output;
  // TODO(cedmundo): Set position and scale from instance
  float3 bPos = float3(0);
  float scale = 0.5;

  float4 vertexPos = float4(quadXYVertices[input.idx], 0.0f, 1.0f);
  float3 f = normalize(viewParams.viewPos - bPos);
  float3 u = normalize(cross(f, float3(0, 1, 0)));
  float3 r = normalize(cross(u, f));

  f *= scale;
  u *= scale;
  r *= scale;

  float4x4 model = {
    { r.x, u.x, f.x, 0 },
    { r.y, u.y, f.y, 0 },
    { r.z, u.z, f.z, 0 },
    { 0, 0, 0, 1 },
  };
  output.pv = viewParams.pv;
  output.position = mul(mul(viewParams.pv, model), vertexPos);
  return output;
}
