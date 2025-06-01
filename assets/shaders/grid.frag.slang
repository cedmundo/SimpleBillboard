struct PSInput {
  float4x4 pv;
  float3 nearPoint;
  float3 farPoint;
  float4 position : SV_Position;
};

struct PSOutput {
  float4 depth : SV_Depth;
  float4 color : SV_Target;
};

float4 grid(float3 fragPos, float scale) {
  float2 coord = fragPos.xz * scale;
  float2 der = fwidth(coord);
  float2 grid = abs(fract(coord - 0.5f) - 0.5f) / der;
  float line = min(grid.x, grid.y);
  float minz = min(der.y, 1);
  float minx = min(der.x, 1);
  float4 color = float4(0.2f, 0.2f, 0.2f, 1.0f - min(line, 1.0f));
  // z axis
  if (fragPos.x > -0.1f * minx && fragPos.x < 0.1f * minx) {
    color.b = 1.0f;
  }

  // x axis
  if (fragPos.z > -0.1f * minz && fragPos.z < 0.1f * minz) {
    color.r = 1.0f;
  }

  return color;
}

float computeDepth(float3 pos, float4x4 pv) {
  float4 clipSpacePos = mul(pv, float4(pos.xyz, 1.0f));
  float ndcDepth = clipSpacePos.z / clipSpacePos.w;  // NDC
  return ndcDepth * 0.5f + 0.5f;                     // [0,1] SV_Depth
}

[shader("pixel")]
PSOutput pixelMain(PSInput input) {
  PSOutput output;
  float t = -input.nearPoint.y / (input.farPoint.y - input.nearPoint.y);
  float3 fragPos = input.nearPoint + t * (input.farPoint - input.nearPoint);
  output.depth = computeDepth(fragPos, input.pv);
  output.color = grid(fragPos, 10.0f) * float(t > 0);
  return output;
}
