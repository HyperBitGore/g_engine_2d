#pragma once
#include <iostream>




template<size_t size>
class vec {
private:
	float *f = nullptr;
	size_t width = 0;
public:
	vec() {
		this->width = size;
		this->f = new float[width];
	}
	//copy constructor
	vec(vec& v) {
		this->width = v.width;
		this->f = new float[this->width];
		std::memcpy(f, v.f, width * sizeof(float));
	}
	//move constructor
	vec(vec&& v) {
		this->width = v.width;
		if (f) {
			delete[] f;
		}
		f = new float[width];
		std::memcpy(f, v.f, width * sizeof(float));
		if (v.f) {
			delete[] v.f;
		}
	}
	~vec() {
		if (f) {
			delete[] f;
		}
	}
	float& operator[](size_t index) {
		return f[index];
	}
	float& operator[](size_t index) const {
		return f[index];
	}

	vec& operator=(const vec& rhs) {
		this->width = rhs.width;
		if (f) {
			delete[] f;
		}
		f = new float[width];
		std::memcpy(f, rhs.f, width * sizeof(float));
		return *this;
	}

	vec& operator*=(const vec& r) {
		for (size_t i = 0; i < width; i++) {
			f[i] *= r.f[i];
		}
		return *this;
	}
	vec& operator+=(const vec& r) {
		for (size_t i = 0; i < width; i++) {
			f[i] += r.f[i];
		}
		return *this;
	}
	vec& operator-=(const vec& r) {
		for (size_t i = 0; i < width; i++) {
			f[i] -= r.f[i];
		}
		return *this;
	}
	vec& operator*=(const float& n) {
		for (size_t i = 0; i < width; i++) {
			f[i] *= n;
		}
		return *this;
	}
	vec& operator^=(const float& n) {
		for (size_t i = 0; i < width; i++) {
			f[i] = std::powf(f[i], n);
		}
		return *this;
	}


	friend vec operator+(vec lhs, const vec& rhs) {
		lhs += rhs;
		return lhs;
	}
	friend vec operator-(vec lhs, const vec& rhs) {
		lhs -= rhs;
		return lhs;
	}
	friend vec operator*(vec lhs, const float& n) {
		lhs *= n;
		return lhs;
	}
	friend vec operator*(vec lhs, const vec& rhs) {
		lhs *= rhs;
		return lhs;
	}
	friend vec operator^(vec lhs, const float& n) {
		lhs ^= n;
		return lhs;
	}
	float* getdata() {
		return f;
	}
};
class vec2 {
public:
	float x;
	float y;
	vec2(float x, float y) {
		this->x = x;
		this->y = y;
	}
	vec2() {
		this->x = 0;
		this->y = 0;
	}
	vec2(const vec2& r) {
		this->x = r.x;
		this->y = r.y;
	}
	vec2& operator=(const vec2& r) {
		this->x = r.x;
		this->y = r.y;
		return *this;
	}
};
class vec3 {
public:
	float x;
	float y;
	float z;
	vec3(float x, float y, float z) {
		this->x = x;
		this->y = y;
		this->z = z;
	}
	vec3() {
		this->x = 0;
		this->y = 0;
		this->z = 0;
	}
	vec3(const vec3& r) {
		this->x = r.x;
		this->y = r.y;
		this->z = r.z;
	}
	vec3& operator=(const vec3& r) {
		this->x = r.x;
		this->y = r.y;
		this->z = r.z;
		return *this;
	}
};
class vec4 {
public:
	float x;
	float y;
	float z;
	float w;
	vec4(float x, float y, float z, float w) {
		this->x = x;
		this->y = y;
		this->z = z;
		this->w = w;
	}
	vec4() {
		this->x = 0;
		this->y = 0;
		this->z = 0;
		this->w = 0;
	}
	vec4(const vec4& r) {
		this->x = r.x;
		this->y = r.y;
		this->z = r.z;
		this->w = r.w;
	}
	vec4& operator=(const vec4& r) {
		this->x = r.x;
		this->y = r.y;
		this->z = r.z;
		this->w = r.w;
		return *this;
	}
};