#ifndef SBI_XMATH_H
#define SBI_XMATH_H

#include <SDL3/SDL_stdinc.h>
#include <stdalign.h>
#define SBI_ALIGN_AS(x) _Alignas(x)
#define SBI_ALIGN_MAT4 SBI_ALIGN_AS(16)
#define SBI_ALIGN_QUAT SBI_ALIGN_AS(16)
#define SBI_ALIGN_VEC4 SBI_ALIGN_AS(16)
#define SBI_ALIGN_VEC3 SBI_ALIGN_AS(8)
#define SBI_ALIGN_XFORM SBI_ALIGN_AS(16)

// Floating point vector of 2 dimensions
typedef float SBI_Vec2[2];

// Floating point vector of 3 dimensions
typedef float SBI_Vec3[3];

// Floating point vector of 4 dimensions
typedef float SBI_Vec4[4];

// Floating point quaternion: ijkr
typedef float SBI_Quat[4];

// Floating point matrix of 4x4 components, column major
typedef float SBI_Mat4[16];

// Floating point transform with rotation:quat,position:vec3,scale:vec3
// components
typedef float SBI_XForm[10];
// TODO(cedmundo): Add API to get the pointers of rot,pos,sca

// Makes a new Vec3 using scalar components
void SBI_Vec3Make(float x, float y, float z, SBI_Vec3 dest);

// Subtract two vec3 into dest
void SBI_Vec3Sub(const SBI_Vec3 a, const SBI_Vec3 b, SBI_Vec3 dest);

// Add two vec3s into dest
void SBI_Vec3Add(const SBI_Vec3 a, const SBI_Vec3 b, SBI_Vec3 dest);

// Multiplies all components by a scalar into dest
void SBI_Vec3Scale(const SBI_Vec3 a, float s, SBI_Vec3 dest);

// Dot product between two vectors
float SBI_Vec3Dot(const SBI_Vec3 a, const SBI_Vec3 b);

// Cross product between two vectors into dest
void SBI_Vec3Cross(const SBI_Vec3 a, const SBI_Vec3 b, SBI_Vec3 dest);

// Length of the given vector
float SBI_Vec3Len(const SBI_Vec3 v);

// Normalizes a vec3 into dest
void SBI_Vec3Normalize(const SBI_Vec3 src, SBI_Vec3 dest);

// Negates a vec3 into dest
void SBI_Vec3Negate(const SBI_Vec3 src, SBI_Vec3 dest);

// Copies values from src into dest
void SBI_Vec3Copy(const SBI_Vec3 src, SBI_Vec3 dest);

// Creates a quaternion using axis and angle into dest
void SBI_QuatMakeAxisAngle(const SBI_Vec3 axis, float angle, SBI_Quat dest);

// Get the angle between two quaternions
float SBI_QuatAngleTo(const SBI_Quat a, const SBI_Quat b);

// Dot product between two quaternions
float SBI_QuatDot(const SBI_Quat a, const SBI_Quat b);

// Squared lenght of a quaternion (l*l), faster than lenght
float SBI_QuatLenSq(const SBI_Quat src);

// Invert a quaternion
void SBI_QuatInvert(const SBI_Quat src, SBI_Quat dest);

// Multiplies two quaternions into dest
void SBI_QuatMul(const SBI_Quat a, const SBI_Quat b, SBI_Quat dest);

// Rotate a vec3 using a quaternion
void SBI_QuatTransformVec3(const SBI_Quat q, const SBI_Vec3 v, SBI_Vec3 dest);

// Create a quaternion looking at a direction
void SBI_QuatLookRotation(const SBI_Vec3 dir, const SBI_Vec3 up, SBI_Quat dest);

// Convert a quaternion into a 4x4 matrix
void SBI_QuatToMat4(const SBI_Quat q, SBI_Mat4 dest);

// Create a prespective matrix
void SBI_Mat4Perspective(float fov,
                         float aspect,
                         float near,
                         float far,
                         SBI_Mat4 dest);

// Resize a perspective matrix using a new aspect ratio
void SBI_Mat4PerspectiveResize(const SBI_Mat4 src, float aspect, SBI_Mat4 dest);

// Invert a matrix
bool SBI_Mat4Invert(const SBI_Mat4 src, SBI_Mat4 dest);

// Multiply two matrices
void SBI_Mat4Mul(const SBI_Mat4 a, const SBI_Mat4 b, SBI_Mat4 dest);

// Convert a 4x4 matrix to a quaternion (basis matrix only)
void SBI_Mat4ToQuat(const SBI_Mat4 m, SBI_Quat dest);

// Transform a Vec4 in the space of mat4 into dest
void SBI_Mat4TransformVec4(const SBI_Mat4 m, const SBI_Vec4 v, SBI_Vec4 dest);

// Initialiaze a transform to identity
void SBI_XFormIdentity(SBI_XForm dest);

// Set the position of a transform to the given vec3
void SBI_XFormTranslate(const SBI_XForm src,
                        const SBI_Vec3 position,
                        SBI_XForm dest);

// Rotate the transform to look at a point in the scene
void SBI_XFormLookAtPoint(const SBI_XForm src,
                          const SBI_Vec3 position,
                          const SBI_Vec3 up,
                          SBI_XForm dest);

// Create a view matrix from an transform
void SBI_XFormToView(const SBI_XForm xform, SBI_Mat4 view);

// Create a model matrix from an transform
void SBI_XFormToModel(const SBI_XForm xform, SBI_Mat4 model);

// Get the position of a transform
void SBI_XFormGetPosition(const SBI_XForm xform, SBI_Vec3 position);

#define SBI_Rads(x) ((x)*0.01745329f)
#endif /* SBI_XMATH_H */
