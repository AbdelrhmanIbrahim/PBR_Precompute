#pragma once

#include "Vector.h"

#include <math.h>

namespace math
{
	/* 
	Xx Yx Zx 0 
	Xy Yy Zy 0
	Xz Yz Zz 0
	Tx Ty Tz 1
	*/
	struct Mat4f
	{
		vec4f data[4];

		vec4f&
		operator[](unsigned int index)
		{
			return data[index];
		}

		const vec4f&
		operator[](unsigned int index) const
		{
			return data[index];
		}

		inline Mat4f
		operator*(const Mat4f& other) const
		{
			return Mat4f 
			{
				data[0] * other[0][0] + data[1] * other[0][1] + data[2] * other[0][2] + data[3] * other[0][3],
				data[0] * other[1][0] + data[1] * other[1][1] + data[2] * other[1][2] + data[3] * other[1][3],
				data[0] * other[2][0] + data[1] * other[2][1] + data[2] * other[2][2] + data[3] * other[2][3],
				data[0] * other[3][0] + data[1] * other[3][1] + data[2] * other[3][2] + data[3] * other[3][3]
			};
		}

		inline vec3f
		operator*(const vec4f& other)
		{
			return vec3f
			{
				data[0][0] * other[0] + data[1][0] * other[1] + data[2][0] * other[2] + data[3][0] * other[3],
				data[0][1] * other[0] + data[1][1] * other[1] + data[2][1] * other[2] + data[3][1] * other[3],
				data[0][2] * other[0] + data[1][2] * other[1] + data[2][2] * other[2] + data[3][2] * other[3]
			};
		}
	};

	inline Mat4f
	mat4_scale(const vec4f& scale)
	{
		Mat4f m{};
		m[0][0] = scale[0];
		m[1][1] = scale[1];
		m[2][2] = scale[2];
		m[3][3] = scale[2];
		return m;
	}

	inline Mat4f
	mat4_scale(const vec3f& scale)
	{
		Mat4f m{};
		m[0][0] = scale[0];
		m[1][1] = scale[1];
		m[2][2] = scale[2];
		m[3][3] = 1;
		return m;
	}

	inline Mat4f
	mat4_id()
	{
		return mat4_scale(vec4f{ 1, 1, 1, 1 });
	}

	inline Mat4f
	mat4_translate(const vec3f& translation)
	{
		Mat4f m = mat4_id();
		m[3] = vec4f{translation[0], translation[1], translation[2], 1.0f};
		return m;
	}

	inline Mat4f
	mat4f_rotate(const vec3f& axis_normal, float rad)
	{
		Mat4f m = mat4_id();
		float cosine = (float)cos(rad);
		float sine = (float)sin(rad);
		float nx = axis_normal[0];
		float ny = axis_normal[1];
		float nz = axis_normal[2];
		vec3f temp = axis_normal * (1.0f - cosine);

		// Rodriguries formula
		m[0][0] = nx * temp[0] + cosine;
		m[0][1] = ny * temp[0] + nz * sine;
		m[0][2] = nz * temp[0] - ny * sine;

		m[1][0] = nx * temp[1] - nz * sine;
		m[1][1] = ny * temp[1] + cosine;
		m[1][2] = nz * temp[1] + nx * sine;

		m[2][0] = nx * temp[2] + ny * sine;
		m[2][1] = ny * temp[2] - nx * sine;
		m[2][2] = nz * temp[2] + cosine;

		return m;
	}

	inline Mat4f
	mat4_transform(const vec3f axis, float angle_rad, const vec3f& scale, const vec3f& translation)
	{
		return mat4_translate(translation) * mat4f_rotate(axis, angle_rad) * mat4_scale(scale);
	}
};