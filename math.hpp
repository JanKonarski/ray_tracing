#include <cmath>

#ifndef MATH_HPP
#define MATH_HPP

const int sceneSize = 5;

struct vector2f_f
{
	union {
		struct {
			float x;
			float y;
		};
	};

	vector2f_f(float x = 0, float y = 0) : x(x), y(y) {}
	vector2f_f(const vector2f_f& v) : x(v.x), y(v.y) {}
};

struct vector3f_t
{
	union {
		struct {
			float x;
			float y;
			float z;
			float w;
		};
	};

	vector3f_t(float _x = 0, float _y = 0, float _z = 0) : x(_x), y(_y), z(_z) {}
	vector3f_t(const vector3f_t& v) : x(v.x), y(v.y), z(v.z) {}

	inline void normalize() {
		float norm = sqrtf(x*x + y*y + z*z);
		x /= norm;
		y /= norm;
		z /= norm;
	}

	inline vector3f_t operator+(const vector3f_t& v) const {
		return vector3f_t(x + v.x,
						  y + v.y,
						  z + v.z);
	}

	inline vector3f_t& operator+=(const vector3f_t& v) {
		x += v.x;
		y += v.y;
		z += v.z;
		return *this;
	}

	inline vector3f_t operator*(float a) const {
		return vector3f_t(x*a,
						  y*a,
						  z*a);
	}

	inline vector3f_t operator*(const vector3f_t& v) const {
		return vector3f_t(x * v.x,
						  y * v.y,
						  z * v.z);
	}
};

inline vector3f_t cross(const vector3f_t& v1, const vector3f_t& v2) {
	return vector3f_t(v1.y*v2.z - v1.z*v2.y,
					  v1.z*v2.x - v1.x*v2.z,
					  v1.x*v2.y - v1.y*v2.x);
	}

struct sphere_t
{
	float radius;
	int m1;
	float m2;
	float m3;
	vector3f_t position;
	vector3f_t color;
	vector3f_t emission;
};

void initScene(sphere_t* cpu_spheres){

	// floor
	cpu_spheres[0].radius = 200.0f;
	cpu_spheres[0].position = vector3f_t(0.0f, -200.4f, 0.0f);
	cpu_spheres[0].color = vector3f_t(1.0f, 1.0f, 1.0f);
	cpu_spheres[0].emission = vector3f_t(0.0f, 0.0f, 0.0f);

	// first sphere
	cpu_spheres[1].radius = 0.16f;
	cpu_spheres[1].position = vector3f_t(-0.25f, -0.24f, -0.1f);
	cpu_spheres[1].color = vector3f_t(0.2f, 0.5f, 0.9f);
	cpu_spheres[1].emission = vector3f_t(0.0f, 0.0f, 0.0f);

	// second sphere
	cpu_spheres[2].radius = 0.14f;
	cpu_spheres[2].position = vector3f_t(0.25f, -0.24f, 0.1f);
	cpu_spheres[2].color = vector3f_t(0.9f, 0.8f, 0.7f);
	cpu_spheres[2].emission = vector3f_t(0.0f, 0.0f, 0.0f);

	// third sphere
	cpu_spheres[3].radius = 0.18f;
	cpu_spheres[3].position = vector3f_t(0.3f, 0.15f, 0.1f);
	cpu_spheres[3].color = vector3f_t(0.5f, 0.7f, 0.2f);
	cpu_spheres[3].emission = vector3f_t(0.0f, 0.0f, 0.0f);

	// lightsource
	cpu_spheres[4].radius = 0.5f;
	cpu_spheres[4].position = vector3f_t(0.0f, 1.35f, 0.0f);
	cpu_spheres[4].color = vector3f_t(0.0f, 0.0f, 0.0f);
	cpu_spheres[4].emission = vector3f_t(15.0f, 15.0f, 15.0f);
}

#endif // MATH_HPP