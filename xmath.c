#include "xmath.h"
#include <SDL3/SDL_assert.h>
#include <SDL3/SDL_stdinc.h>

#define XX 0
#define XY 1
#define XZ 2
#define XW 3

#define YX 4
#define YY 5
#define YZ 6
#define YW 7

#define ZX 8
#define ZY 9
#define ZZ 10
#define ZW 11

#define WX 12
#define WY 13
#define WZ 14
#define WW 15

void SBI_Vec3Make(float x, float y, float z, SBI_Vec3 dest) {
  dest[0] = x;
  dest[1] = y;
  dest[2] = z;
}

void SBI_Vec3Sub(const SBI_Vec3 a, const SBI_Vec3 b, SBI_Vec3 dest) {
  dest[0] = a[0] - b[0];
  dest[1] = a[1] - b[1];
  dest[2] = a[2] - b[2];
}

void SBI_Vec3Add(const SBI_Vec3 a, const SBI_Vec3 b, SBI_Vec3 dest) {
  dest[0] = a[0] + b[0];
  dest[1] = a[1] + b[1];
  dest[2] = a[2] + b[2];
}

void SBI_Vec3Scale(const SBI_Vec3 a, float s, SBI_Vec3 dest) {
  dest[0] = a[0] * s;
  dest[1] = a[1] * s;
  dest[2] = a[2] * s;
}

float SBI_Vec3Dot(const SBI_Vec3 a, const SBI_Vec3 b) {
  return a[0] * b[0] + a[1] * b[1] + a[2] * b[2];
}

void SBI_Vec3Cross(const SBI_Vec3 a, const SBI_Vec3 b, SBI_Vec3 dest) {
  dest[0] = a[1] * b[2] - a[2] * b[1];
  dest[1] = a[2] * b[0] - a[0] * b[2];
  dest[2] = a[0] * b[1] - a[1] * b[0];
}

void SBI_Vec3Copy(const SBI_Vec3 src, SBI_Vec3 dest) {
  dest[0] = src[0];
  dest[1] = src[1];
  dest[2] = src[2];
}

float SBI_Vec3LenSq(const SBI_Vec3 v) {
  return SBI_Vec3Dot(v, v);
}

float SBI_Vec3Len(const SBI_Vec3 v) {
  return SDL_sqrtf(SBI_Vec3LenSq(v));
}

void SBI_Vec3Normalize(const SBI_Vec3 src, SBI_Vec3 dest) {
  float n = SBI_Vec3Len(src);
  if (n < SDL_FLT_EPSILON) {
    dest[0] = 0.0f;
    dest[1] = 0.0f;
    dest[2] = 0.0f;
    return;
  }

  SBI_Vec3Scale(src, 1.0f / n, dest);
}

void SBI_Vec3Negate(const SBI_Vec3 src, SBI_Vec3 dest) {
  dest[0] = -src[0];
  dest[1] = -src[1];
  dest[2] = -src[2];
}

float SBI_QuatAngleTo(const SBI_Quat a, const SBI_Quat b) {
  float d = SBI_QuatDot(a, b);
  return SDL_acosf(d * d * 2.0f - 1.0f);
}

float SBI_QuatDot(const SBI_Quat a, const SBI_Quat b) {
  return a[0] * b[0] + a[1] * b[1] + a[2] * b[2] + a[3] * b[3];
}

float SBI_QuatLenSq(const SBI_Quat src) {
  return SBI_QuatDot(src, src);
}

void SBI_QuatInvert(const SBI_Quat src, SBI_Quat dest) {
  float l_sq = SBI_QuatLenSq(src);
  if (l_sq < SDL_FLT_EPSILON) {
    dest[0] = 0.0f;
    dest[1] = 0.0f;
    dest[2] = 0.0f;
    dest[3] = 1.0f;
    return;
  }

  float recip = 1.0f / l_sq;
  dest[0] = -src[0] * recip;
  dest[1] = -src[1] * recip;
  dest[2] = -src[2] * recip;
  dest[3] = src[3] * recip;
}

void SBI_QuatMakeAxisAngle(const SBI_Vec3 axis, float angle, SBI_Quat dest) {
  SBI_ALIGN_VEC3 SBI_Vec3 n_axis = {0};
  float l = SBI_Vec3Len(axis);
  if (SDL_fabsf(l) < SDL_FLT_EPSILON) {
    dest[0] = 0.0f;
    dest[1] = 0.0f;
    dest[2] = 0.0f;
    dest[3] = 0.0f;
    return;
  }

  SBI_Vec3Normalize(axis, n_axis);
  float s = SDL_sinf(angle * 0.5f) / l;
  dest[0] = n_axis[0] * s;
  dest[1] = n_axis[1] * s;
  dest[2] = n_axis[2] * s;
  dest[3] = SDL_cosf(angle * 0.5f);
}

void SBI_QuatTransformVec3(const SBI_Quat q, const SBI_Vec3 v, SBI_Vec3 dest) {
  SBI_ALIGN_VEC3 SBI_Vec3 a = {0};
  SBI_ALIGN_VEC3 SBI_Vec3 b = {0};
  SBI_ALIGN_VEC3 SBI_Vec3 c = {0};
  SBI_ALIGN_VEC3 SBI_Vec3 i = {q[0], q[1], q[2]};
  float s = q[3];

  SBI_Vec3Scale(i, 2.0f * SBI_Vec3Dot(i, v), a);
  SBI_Vec3Scale(v, s * s - SBI_Vec3Dot(i, i), b);
  SBI_Vec3Cross(i, v, c);
  SBI_Vec3Scale(c, 2.0f * s, c);
  SBI_Vec3Add(a, b, dest);
  SBI_Vec3Add(dest, c, dest);
}

void SBI_QuatLookRotation(const SBI_Vec3 dir,
                          const SBI_Vec3 up,
                          SBI_Quat dest) {
  SBI_ALIGN_MAT4 SBI_Mat4 m = {0};
  SBI_ALIGN_VEC3 SBI_Vec3 a = {0};
  SBI_ALIGN_VEC3 SBI_Vec3 b = {0};
  SBI_ALIGN_VEC3 SBI_Vec3 c = {0};

  SBI_Vec3Scale(dir, -1, c);
  SBI_Vec3Cross(up, c, a);
  SBI_Vec3Normalize(a, a);
  SBI_Vec3Cross(c, a, b);

  m[XX] = a[0];
  m[XY] = a[1];
  m[XZ] = a[2];
  m[XW] = 0.0f;

  m[YX] = b[0];
  m[YY] = b[1];
  m[YZ] = b[2];
  m[YW] = 0.0f;

  m[ZX] = c[0];
  m[ZY] = c[1];
  m[ZZ] = c[2];
  m[ZW] = 0.0f;

  m[WX] = 0.0f;
  m[WY] = 0.0f;
  m[WZ] = 0.0f;
  m[WW] = 1.0f;

  SBI_Mat4ToQuat(m, dest);
}

void SBI_QuatToMat4(const SBI_Quat q, SBI_Mat4 dest) {
  SBI_ALIGN_VEC3 SBI_Vec3 w_r = {1.0f, 0.0f, 0.0f};
  SBI_ALIGN_VEC3 SBI_Vec3 w_u = {0.0f, 1.0f, 0.0f};
  SBI_ALIGN_VEC3 SBI_Vec3 w_f = {0.0f, 0.0f, 1.0f};

  SBI_ALIGN_VEC3 SBI_Vec3 l_r = {0};
  SBI_ALIGN_VEC3 SBI_Vec3 l_u = {0};
  SBI_ALIGN_VEC3 SBI_Vec3 l_f = {0};
  SBI_QuatTransformVec3(q, w_r, l_r);
  SBI_QuatTransformVec3(q, w_u, l_u);
  SBI_QuatTransformVec3(q, w_f, l_f);

  dest[XX] = l_r[0];
  dest[XY] = l_u[0];
  dest[XZ] = l_f[0];
  dest[XW] = 0.0f;

  dest[YX] = l_r[1];
  dest[YY] = l_u[1];
  dest[YZ] = l_f[1];
  dest[YW] = 0.0f;

  dest[ZX] = l_r[2];
  dest[ZY] = l_u[2];
  dest[ZZ] = l_f[2];
  dest[ZW] = 0.0f;

  dest[WX] = 0.0f;
  dest[WY] = 0.0f;
  dest[WZ] = 0.0f;
  dest[WW] = 1.0f;
}

void SBI_Mat4Perspective(float fov,
                         float aspect,
                         float near,
                         float far,
                         SBI_Mat4 dest) {
  float const e = 1.0f / SDL_tanf(fov / 2.0f);
  float const f = 1.0f / (near - far);

  dest[XX] = e / aspect;
  dest[XY] = 0.0f;
  dest[XZ] = 0.0f;
  dest[XW] = 0.0f;

  dest[YX] = 0.0f;
  dest[YY] = e;
  dest[YZ] = 0.0f;
  dest[YW] = 0.0f;

  dest[ZX] = 0.0f;
  dest[ZY] = 0.0f;
  dest[ZZ] = far * f;
  dest[ZW] = -1.0f;

  dest[WX] = 0.0f;
  dest[WY] = 0.0f;
  dest[WZ] = near * far * f;
  dest[WW] = 0.0f;
}

void SBI_Mat4PerspectiveResize(const SBI_Mat4 src,
                               float aspect,
                               SBI_Mat4 dest) {
  if (src[XX] == 0.0f) {
    return;
  }

  dest[XX] = dest[YY] / aspect;
}

bool SBI_Mat4Invert(const SBI_Mat4 src, SBI_Mat4 dest) {
  float a = src[XX], b = src[XY], c = src[XZ], d = src[XW];
  float e = src[YX], f = src[YY], g = src[YZ], h = src[YW];
  float i = src[ZX], j = src[ZY], k = src[ZZ], l = src[ZW];
  float m = src[WX], n = src[WY], o = src[WZ], p = src[WW];

  float c1 = k * p - l * o, c2 = c * h - d * g, c3 = i * p - l * m;
  float c4 = a * h - d * e, c5 = j * p - l * n, c6 = b * h - d * f;
  float c7 = i * n - j * m, c8 = a * f - b * e, c9 = j * o - k * n;
  float c10 = b * g - c * f, c11 = i * o - k * m, c12 = a * g - c * e;

  float det = (c8 * c1 + c4 * c9 + c10 * c3 + c2 * c7 - c12 * c5 - c6 * c11);
  if (det == 0) {
    return false;
  }

  float idt = 1.0f / det;
  float ndt = -idt;

  dest[XX] = (f * c1 - g * c5 + h * c9) * idt;
  dest[XY] = (b * c1 - c * c5 + d * c9) * ndt;
  dest[XZ] = (n * c2 - o * c6 + p * c10) * idt;
  dest[XW] = (j * c2 - k * c6 + l * c10) * ndt;

  dest[YX] = (e * c1 - g * c3 + h * c11) * ndt;
  dest[YY] = (a * c1 - c * c3 + d * c11) * idt;
  dest[YZ] = (m * c2 - o * c4 + p * c12) * ndt;
  dest[YW] = (i * c2 - k * c4 + l * c12) * idt;

  dest[ZX] = (e * c5 - f * c3 + h * c7) * idt;
  dest[ZY] = (a * c5 - b * c3 + d * c7) * ndt;
  dest[ZZ] = (m * c6 - n * c4 + p * c8) * idt;
  dest[ZW] = (i * c6 - j * c4 + l * c8) * ndt;

  dest[WX] = (e * c9 - f * c11 + g * c7) * ndt;
  dest[WY] = (a * c9 - b * c11 + c * c7) * idt;
  dest[WZ] = (m * c10 - n * c12 + o * c8) * ndt;
  dest[WW] = (i * c10 - j * c12 + k * c8) * idt;
  return true;
}

void SBI_Mat4Mul(const SBI_Mat4 m1, const SBI_Mat4 m2, SBI_Mat4 dest) {
  float a00 = m1[XX], a01 = m1[XY], a02 = m1[XZ], a03 = m1[XW];
  float a10 = m1[YX], a11 = m1[YY], a12 = m1[YZ], a13 = m1[YW];
  float a20 = m1[ZX], a21 = m1[ZY], a22 = m1[ZZ], a23 = m1[ZW];
  float a30 = m1[WX], a31 = m1[WY], a32 = m1[WZ], a33 = m1[WW];

  float b00 = m2[XX], b01 = m2[XY], b02 = m2[XZ], b03 = m2[XW];
  float b10 = m2[YX], b11 = m2[YY], b12 = m2[YZ], b13 = m2[YW];
  float b20 = m2[ZX], b21 = m2[ZY], b22 = m2[ZZ], b23 = m2[ZW];
  float b30 = m2[WX], b31 = m2[WY], b32 = m2[WZ], b33 = m2[WW];

  dest[XX] = a00 * b00 + a10 * b01 + a20 * b02 + a30 * b03;
  dest[XY] = a01 * b00 + a11 * b01 + a21 * b02 + a31 * b03;
  dest[XZ] = a02 * b00 + a12 * b01 + a22 * b02 + a32 * b03;
  dest[XW] = a03 * b00 + a13 * b01 + a23 * b02 + a33 * b03;
  dest[YX] = a00 * b10 + a10 * b11 + a20 * b12 + a30 * b13;
  dest[YY] = a01 * b10 + a11 * b11 + a21 * b12 + a31 * b13;
  dest[YZ] = a02 * b10 + a12 * b11 + a22 * b12 + a32 * b13;
  dest[YW] = a03 * b10 + a13 * b11 + a23 * b12 + a33 * b13;
  dest[ZX] = a00 * b20 + a10 * b21 + a20 * b22 + a30 * b23;
  dest[ZY] = a01 * b20 + a11 * b21 + a21 * b22 + a31 * b23;
  dest[ZZ] = a02 * b20 + a12 * b21 + a22 * b22 + a32 * b23;
  dest[ZW] = a03 * b20 + a13 * b21 + a23 * b22 + a33 * b23;
  dest[WX] = a00 * b30 + a10 * b31 + a20 * b32 + a30 * b33;
  dest[WY] = a01 * b30 + a11 * b31 + a21 * b32 + a31 * b33;
  dest[WZ] = a02 * b30 + a12 * b31 + a22 * b32 + a32 * b33;
  dest[WW] = a03 * b30 + a13 * b31 + a23 * b32 + a33 * b33;
}

void SBI_Mat4ToQuat(const SBI_Mat4 m, SBI_Quat dest) {
  float fx = m[XX] - m[YY] - m[ZZ];
  float fy = m[YY] - m[XX] - m[ZZ];
  float fz = m[ZZ] - m[XX] - m[YY];
  float fw = m[XX] + m[YY] + m[ZZ];

  Uint32 bi = 0;
  float fb = fw;
  if (fx > fb) {
    fb = fx;
    bi = 1;
  }

  if (fy > fb) {
    fb = fy;
    bi = 2;
  }

  if (fz > fb) {
    fb = fz;
    bi = 3;
  }

  float bv = SDL_sqrtf(fb + 1.0f) * 0.5f;
  float fm = 0.25f / bv;
  switch (bi) {
    case 0:
      dest[0] = (m[YZ] - m[ZY]) * fm;
      dest[1] = (m[ZX] - m[XZ]) * fm;
      dest[2] = (m[XY] - m[YX]) * fm;
      dest[3] = bv;
      break;
    case 1:
      dest[0] = bv;
      dest[1] = (m[XY] + m[YX]) * fm;
      dest[2] = (m[ZX] + m[XZ]) * fm;
      dest[3] = (m[YZ] - m[ZY]) * fm;
      break;
    case 2:
      dest[0] = (m[XY] + m[YX]) * fm;
      dest[1] = bv;
      dest[2] = (m[YZ] + m[ZY]) * fm;
      dest[3] = (m[ZX] - m[XZ]) * fm;
      break;
    case 3:
      dest[0] = (m[ZX] + m[XZ]) * fm;
      dest[1] = (m[YZ] + m[ZY]) * fm;
      dest[2] = bv;
      dest[3] = (m[XY] - m[YX]) * fm;
      break;
    default:
      SDL_assert(false && "unreachable code: undefined biggest index");
  }
}

void SBI_Mat4TransformVec4(const SBI_Mat4 m, const SBI_Vec4 v, SBI_Vec4 dest) {
  dest[0] = m[XX] * v[0] + m[YX] * v[1] + m[ZX] * v[2] + m[WX] * v[3];
  dest[1] = m[XY] * v[0] + m[YY] * v[1] + m[ZY] * v[2] + m[WY] * v[3];
  dest[2] = m[XZ] * v[0] + m[YZ] * v[1] + m[ZZ] * v[2] + m[WZ] * v[3];
  dest[3] = m[XW] * v[0] + m[YW] * v[1] + m[ZW] * v[2] + m[WW] * v[3];
}

void SBI_XFormIdentity(SBI_XForm dest) {
  // rotation (quat)
  dest[0] = 0.0f;
  dest[1] = 0.0f;
  dest[2] = 0.0f;
  dest[3] = 1.0f;

  // position (vec3)
  dest[4] = 0.0f;
  dest[5] = 0.0f;
  dest[6] = 0.0f;

  // scale (vec3)
  dest[7] = 1.0f;
  dest[8] = 1.0f;
  dest[9] = 1.0f;
}

void SBI_XFormTranslate(const SBI_XForm src,
                        const SBI_Vec3 position,
                        SBI_XForm dest) {
  // rotation (quat)
  dest[0] = src[0];
  dest[1] = src[1];
  dest[2] = src[2];
  dest[3] = src[3];

  // position (vec3)
  dest[4] = position[0];
  dest[5] = position[1];
  dest[6] = position[2];

  // scale (vec3)
  dest[7] = src[7];
  dest[8] = src[8];
  dest[9] = src[9];
}

void SBI_XFormLookAtPoint(const SBI_XForm src,
                          const SBI_Vec3 position,
                          const SBI_Vec3 up,
                          SBI_XForm dest) {
  SBI_ALIGN_VEC3 SBI_Vec3 dir = {0};
  SBI_Vec3Sub(position, &src[4], dir);
  SBI_Vec3Normalize(dir, dir);
  SBI_QuatLookRotation(dir, up, &dest[0]);
}

void SBI_XFormToView(const SBI_XForm xform, SBI_Mat4 view) {
  SBI_ALIGN_VEC4 SBI_Vec4 pos = {xform[4], xform[5], xform[6], 1.0f};
  SBI_QuatToMat4(xform, view);
  SBI_Mat4TransformVec4(view, pos, &view[WX]);
  SBI_Vec3Negate(&view[WX], &view[WX]);
}

void SBI_XFormGetPosition(const SBI_XForm xform, SBI_Vec3 position) {
  SBI_Vec3Copy(&xform[4], position);
}
