#pragma once
#include <iostream>
#include <cstring>
#include <cmath>

namespace gore {
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
	// assumes the length of r is the same as the size you template class
	vec(std::initializer_list<float> r) {
		this->width = size;
		this->f = new float[width];
		size_t i = 0;
		for (float val : r) {
			this->f[i++] = val;
		}
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
	float dotProduct (const vec<size>& r) {
		float f = 0.0f;
		for (size_t i = 0; i < width; i++) {
			f += this->f[i] * r[i];
		}
		return f;
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
	vec operator-() const noexcept {
		vec<size> n_vec;
		for (size_t i = 0; i < width; i++) {
			n_vec[i] = -f[i];
		}
		return n_vec;
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

	float dotProduct (const gore::vec2& r) {
		float f = x * r.x;
		f += y * r.y;
		return f;
	} 

	vec2& operator=(const vec2& r) {
		this->x = r.x;
		this->y = r.y;
		return *this;
	}
	vec2& operator+=(const vec2& r) {
		x += r.x;
		y += r.y;
		return *this;
	}
	vec2& operator-=(const vec2& r) {
		x -= r.x;
		y -= r.y;
		return *this;
	}
	//multiplication functions
	vec2& operator*=(const vec2& r) {
		x *= r.x;
		y *= r.y;
		return *this;
	}
	/*vec2& operator*=(const vec3& r) {
		x *= r.x;
		y *= r.y;
	}
	vec2& operator*=(const vec4& r) {
		x *= r.x;
		y *= r.y;
	}*/
	vec2& operator*=(const float& n) {
		x *= n;
		y *= n;
		return *this;
	}
	vec2& operator^=(const float& n) {
		x = std::powf(x, n);
		y = std::powf(y, n);
		return *this;
	}


	friend vec2 operator+(vec2 lhs, const vec2& rhs) {
		lhs += rhs;
		return lhs;
	}
	friend vec2 operator-(vec2 lhs, const vec2& rhs) {
		lhs -= rhs;
		return lhs;
	}
	friend vec2 operator*(vec2 lhs, const float& n) {
		lhs *= n;
		return lhs;
	}
	friend vec2 operator*(vec2 lhs, const vec2& rhs) {
		lhs *= rhs;
		return lhs;
	}
	/*friend vec2 operator*(vec2 lhs, const vec3& rhs) {
		lhs *= rhs;
		return lhs;
	}
	friend vec2 operator*(vec2 lhs, const vec4& rhs) {
		lhs *= rhs;
		return lhs;
	}*/
	friend vec2 operator^(vec2 lhs, const float& n) {
		lhs ^= n;
		return lhs;
	}
	vec2 operator-() const noexcept {
		return {-x, -y};
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
	float dotProduct (const gore::vec3& r) {
		float f = x * r.x;
		f += y * r.y;
		f += z * r.z;
		return f;
	} 
	vec3& operator=(const vec3& r) {
		this->x = r.x;
		this->y = r.y;
		this->z = r.z;
		return *this;
	}

	vec3& operator+=(const vec3& r) {
		x += r.x;
		y += r.y;
		z += r.z;
		return *this;
	}
	vec3& operator-=(const vec3& r) {
		x -= r.x;
		y -= r.y;
		z -= r.z;
		return *this;
	}
	//multiplication functions
	vec3& operator*=(const vec3& r) {
		x *= r.x;
		y *= r.y;
		z *= r.z;
		return *this;
	}
	vec3& operator*=(const float& n) {
		x *= n;
		y *= n;
		z *= n;
		return *this;
	}
	vec3& operator*=(const vec2& r) {
		x *= r.x;
		y *= r.y;
		return *this;
	}
	/*vec3& operator*=(const vec4& r) {
		x *= r.x;
		y *= r.y;
		z *= r.z;
		return *this;
	}*/
	vec3& operator^=(const float& n) {
		x = std::powf(x, n);
		y = std::powf(y, n);
		z = std::powf(z, n);
		return *this;
	}


	friend vec3 operator+(vec3 lhs, const vec3& rhs) {
		lhs += rhs;
		return lhs;
	}
	friend vec3 operator-(vec3 lhs, const vec3& rhs) {
		lhs -= rhs;
		return lhs;
	}
	friend vec3 operator*(vec3 lhs, const float& n) {
		lhs *= n;
		return lhs;
	}
	friend vec3 operator*(vec3 lhs, const vec3& rhs) {
		lhs *= rhs;
		return lhs;
	}
	friend vec3 operator*(vec3 lhs, const vec2& rhs) {
		lhs *= rhs;
		return lhs;
	}
	friend vec3 operator*(const vec2& lhs, vec3 rhs) {
		rhs *= lhs;
		return rhs;
	}
	/*friend vec3 operator*(vec3 lhs, const vec4& rhs) {
		lhs *= rhs;
		return lhs;
	}*/
	friend vec3 operator^(vec3 lhs, const float& n) {
		lhs ^= n;
		return lhs;
	}
	vec3 operator-() const noexcept {
		return {-x, -y, -z};
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
	float dotProduct (const gore::vec4& r) {
		float f = x * r.x;
		f += y * r.y;
		f += z * r.z;
		f += w * r.w;
		return f;
	} 
	vec4& operator=(const vec4& r) {
		this->x = r.x;
		this->y = r.y;
		this->z = r.z;
		this->w = r.w;
		return *this;
	}

	vec4& operator+=(const vec4& r) {
		x += r.x;
		y += r.y;
		z += r.z;
		w += r.w;
		return *this;
	}
	vec4& operator-=(const vec4& r) {
		x -= r.x;
		y -= r.y;
		z -= r.z;
		w -= r.w;
		return *this;
	}
	//multiplication functions
	vec4& operator*=(const vec4& r) {
		x *= r.x;
		y *= r.y;
		z *= r.z;
		w *= r.w;
		return *this;
	}
	vec4& operator*=(const float& n) {
		x *= n;
		y *= n;
		z *= n;
		w *= n;
		return *this;
	}
	vec4& operator*=(const vec3& r) {
		x *= r.x;
		y *= r.y;
		z *= r.z;
		return *this;
	}
	vec4& operator*=(const vec2& r) {
		x *= r.x;
		y *= r.y;
		return *this;
	}

	vec4& operator^=(const float& n) {
		x = std::powf(x, n);
		y = std::powf(y, n);
		z = std::powf(z, n);
		w = std::powf(w, n);
		return *this;
	}


	friend vec4 operator+(vec4 lhs, const vec4& rhs) {
		lhs += rhs;
		return lhs;
	}
	friend vec4 operator-(vec4 lhs, const vec4& rhs) {
		lhs -= rhs;
		return lhs;
	}
	friend vec4 operator*(vec4 lhs, const float& n) {
		lhs *= n;
		return lhs;
	}
	friend vec4 operator*(vec4 lhs, const vec3& rhs) {
		lhs *= rhs;
		return lhs;
	}
	friend vec4 operator*(vec4 lhs, const vec2& rhs) {
		lhs *= rhs;
		return lhs;
	}
	friend vec4 operator*(const vec3& lhs, vec4 rhs) {
		rhs *= rhs;
		return rhs;
	}
	friend vec4 operator*(const vec2& lhs, vec4 rhs) {
		rhs *= rhs;
		return rhs;
	}
	friend vec4 operator*(vec4 lhs, const vec4& rhs) {
		lhs *= rhs;
		return lhs;
	}
	friend vec4 operator^(vec4 lhs, const float& n) {
		lhs ^= n;
		return lhs;
	}
	vec4 operator-() const noexcept {
		return {-x, -y, -z, -w};
	}
};
};