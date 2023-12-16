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

	}
	~vec() {
		if (f) {
			delete[] f;
		}
	}
	float& operator[](size_t index) {
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

};
class vec2 {
private:
	vec<2> vecc;
public:
	float& x = vecc[0];
	float& y = vecc[1];
	vec2(float x, float y) {
		this->x = x;
		this->y = y;
	}
	vec2() {
		x = vecc[0];
		y = vecc[1];
	}
	vec2(const vec2& r) {
		this->vecc = r.vecc;
		x = vecc[0];
		y = vecc[1];
	}
	vec2& operator=(const vec2& r) {
		this->vecc = r.vecc;
		this->x = vecc[0];
		this->y = vecc[1];
		return *this;
	}
};
class vec3 {
private:
	vec<3> vecc;
public:
	float& x = vecc[0];
	float& y = vecc[1];
	float& z = vecc[2];
	vec3(float x, float y, float z) {
		this->x = x;
		this->y = y;
		this->z = z;
	}
	vec3() {
		x = vecc[0];
		y = vecc[1];
		z = vecc[2];
	}
	vec3(vec3& r) {
		this->vecc = r.vecc;
		x = vecc[0];
		y = vecc[1];
		z = vecc[2];
	}
	vec3& operator=(const vec3& r) {
		this->vecc = r.vecc;
		this->x = vecc[0];
		this->y = vecc[1];
		this->z = vecc[2];
		return *this;
	}
};
class vec4 {
private:
	vec<4> vecc;
public:
	float& x = vecc[0];
	float& y = vecc[1];
	float& z = vecc[2];
	float& w = vecc[3];
	vec4(float x, float y, float z, float w) {
		x = vecc[0];
		y = vecc[1];
		z = vecc[2];
		w = vecc[3];
		this->x = x;
		this->y = y;
		this->z = z;
		this->w = w;
	}
	vec4() {
		x = vecc[0];
		y = vecc[1];
		z = vecc[2];
		w = vecc[3];
	}
	vec4(vec4& r) {
		this->vecc = r.vecc;
		x = vecc[0];
		y = vecc[1];
		z = vecc[2];
		w = vecc[3];
	}
	vec4& operator=(const vec4& r) {
		this->vecc = r.vecc;
		this->x = vecc[0];
		this->y = vecc[1];
		this->z = vecc[2];
		this->w = vecc[3];
		return *this;
	}
};