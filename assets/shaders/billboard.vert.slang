struct ViewParams {
  float4x4 pv;
  float3 viewPos;
};

struct BillboardInstance {
  float4 ps;
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
  float3 bPos = float3(instance.ps.x, instance.ps.y, instance.ps.z);
  float scale = instance.ps.w;

  float4 vertexPos = float4(quadXYVertices[input.vertexID], 0.0f, 1.0f);
  float3 f = normalize(viewParams.viewPos - bPos);
  float3 u = normalize(cross(f, float3(0, 1, 0)));
  float3 r = normalize(cross(u, f));

  f *= scale;
  u *= scale;
  r *= scale;

  float4x4 model = {
    { r.x, u.x, f.x, bPos.x },
    { r.y, u.y, f.y, bPos.y },
    { r.z, u.z, f.z, bPos.z },
    { 0, 0, 0, 1 },
  };
  output.pv = viewParams.pv;
  output.position = mul(mul(viewParams.pv, model), vertexPos);
  return output;
}
