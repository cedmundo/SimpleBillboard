struct ViewParams {
  float4x4 pv;
  float3 viewPos;
};

struct BillboardInstance {
  float3 position;
  float scale;
};

struct VSInput {
  uint vertexID : SV_VertexID;
  uint instanceID : SV_InstanceID;
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

layout(set = 0, binding = 0) StructuredBuffer<BillboardInstance> instances;
layout(set = 1, binding = 0) ConstantBuffer<ViewParams> viewParams;

[shader("vertex")]
VSOutput vertexMain(VSInput input) {
  VSOutput output;
  BillboardInstance instance = instances[input.instanceID];
  float3 instancePos = instance.position;
  float instanceScale = instance.scale;

  float4 vertexPos = float4(quadXYVertices[input.vertexID], 0.0f, 1.0f);
  float3 f = normalize(viewParams.viewPos - instancePos);
  float3 r = normalize(cross(float3(0, 1, 0), f));
  float3 u = normalize(cross(f, r));

  u *= instanceScale;
  r *= instanceScale;

  float4x4 model = {
    { r.x, u.x, f.x, instancePos.x },
    { r.y, u.y, f.y, instancePos.y },
    { r.z, u.z, f.z, instancePos.z },
    { 0, 0, 0, 1 },
  };
  output.pv = viewParams.pv;
  output.position = mul(mul(viewParams.pv, model), vertexPos);
  return output;
}
