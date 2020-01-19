#pragma once

namespace math
{
	struct vec2f
	{
		float data[2];

		float&
		operator[](unsigned int index)
		{
			return data[index];
		}

		const float&
		operator[](unsigned int index) const
		{
			return data[index];
		}

		inline vec2f
		operator*(float factor) const
		{
			return vec2f{ data[0] * factor, data[1] * factor};
		}
	};

	struct vec3f
	{
		float data[3];

		inline float&
		operator[](unsigned int index)
		{
			return data[index];
		}
		
		inline const float&
		operator[](unsigned int index) const
		{
			return data[index];
		}

		inline vec3f
		operator+(const vec3f& other) const
		{
			return vec3f{ data[0] + other.data[0], data[1] + other.data[1], data[2] + other.data[2] };
		}

		inline vec3f
		operator-(const vec3f& other) const
		{
			return vec3f{ data[0] - other.data[0], data[1] - other.data[1], data[2] - other.data[2] };
		}

		inline vec3f
		operator-() const
		{
			return vec3f{ -data[0], -data[1], -data[2]};
		}

		inline vec3f
		operator*(const vec3f& other) const
		{
			return vec3f{ other.data[0]*data[0], other.data[1]*data[1], other.data[2]*data[2] };
		}

		inline vec3f&
		operator-=(const vec3f& vec)
		{
			data[0] -= vec[0];
			data[1] -= vec[1];
			data[2] -= vec[2];
			return *this;
		}

		inline vec3f&
		operator+=(const vec3f& vec)
		{
			data[0] += vec[0];
			data[1] += vec[1];
			data[2] += vec[2];
			return *this;
		}

		inline vec3f
		operator*(const float factor) const
		{
			return vec3f{ factor*data[0], factor*data[1], factor*data[2] };
		}

		inline vec3f
		operator/(const float factor) const
		{
			return vec3f{ data[0]/factor, data[1]/factor, data[2]/factor };
		}

		inline vec3f&
		operator*=(const float factor)
		{
			data[0] *= factor;
			data[1] *= factor;
			data[2] *= factor;
			return *this;
		}

		inline vec3f&
		operator+=(const float factor)
		{
			data[0] += factor;
			data[1] += factor;
			data[2] += factor;
			return *this;
		}
	};

	struct vec4f
	{
		float data[4];

		inline float&
		operator[](unsigned int index)
		{
			return data[index];
		}

		inline const float&
		operator[](unsigned int index) const
		{
			return data[index];
		}

		inline vec4f
		operator+(const vec4f& other) const
		{
			return vec4f{ data[0] + other.data[0], data[1] + other.data[1], data[2] + other.data[2], data[3] + other.data[3] };
		}

		inline vec4f
		operator-(const vec4f& other) const
		{
			return vec4f{ data[0] - other.data[0], data[1] - other.data[1], data[2] - other.data[2], data[3] - other.data[3] };
		}

		inline vec4f
		operator-() const
		{
			return vec4f{ -data[0], -data[1], -data[2], -data[3] };
		}

		inline vec4f
		operator*(const vec4f& other) const
		{
			return vec4f{ data[0] * other.data[0], data[1] * other.data[1], data[2] * other.data[2], data[3] * other.data[3] };
		}

		inline vec4f
		operator*(const float factor) const
		{
			return vec4f{ factor*data[0], factor*data[1], factor*data[2], factor*data[3] };
		}

		inline vec4f
		operator/(const float factor) const
		{
			return vec4f{ data[0] / factor, data[1] / factor, data[2] / factor, data[3]/factor };
		}

		inline vec4f&
		operator*=(const float factor)
		{
			data[0] *= factor;
			data[1] *= factor;
			data[2] *= factor;
			data[3] *= factor;
			return *this;
		}

		inline vec4f&
		operator+=(const float factor)
		{
			data[0] += factor;
			data[1] += factor;
			data[2] += factor;
			data[3] += factor;
			return *this;
		}
	};

	//lhs
	inline bool
	operator==(const vec2f& first, const vec2f& second)
	{
		return first.data[0] == second.data[0] && first.data[1] == second.data[1];
	}

	inline bool
	operator==(const vec3f& first, const vec3f& second)
	{
		return first.data[0] == second.data[0] && first.data[1] == second.data[1] && first.data[2] == second.data[2];
	}

	inline bool
	operator==(const vec4f& first, const vec4f& second)
	{
		return first.data[0] == second.data[0] && first.data[1] == second.data[1] && first.data[2] == second.data[2] && first.data[3] == second.data[3];
	}

	inline vec2f
	operator*(float factor, const vec2f& vec)
	{
		return vec * factor;
	}

	inline vec3f
	operator*(float factor, const vec3f& vec)
	{
		return vec * factor;
	}

	inline vec4f
	operator*(float factor, const vec4f& vec)
	{
		return vec * factor;
	}

	constexpr vec3f X_AXIS = vec3f{ 1.0f, 0.0f, 0.0f };
	constexpr vec3f Y_AXIS = vec3f{ 0.0f, 1.0f, 0.0f };
	constexpr vec3f Z_AXIS = vec3f{ 0.0f, 0.0f, 1.0f };
}