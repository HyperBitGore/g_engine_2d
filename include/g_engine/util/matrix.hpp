#pragma once
#include "vector.hpp"
#include <cstdint>
#include <string>

//https://en.wikipedia.org/wiki/Matrix_(mathematics)
//https://en.wikipedia.org/wiki/Computational_complexity_of_matrix_multiplication
//https://github.com/g-truc/glm
// -implement perspective, translate, rotate, and scale
//https://www.khanacademy.org/math/linear-algebra/matrix-transformations/lin-trans-examples/v/linear-transformation-examples-scaling-and-reflections
//https://matrixcalc.org/
namespace gore {
class matrix {
private:
	size_t columns;
	size_t rows;
	float* dat;
public:
	matrix() = delete;
	matrix(size_t r, size_t c);
	//copy constructor
	matrix(const matrix& m);
	~matrix();

	matrix& operator=(const matrix& rhs);
	matrix& operator+=(const matrix& rhs);
	matrix& operator-=(const matrix& rhs);
	matrix& operator*=(const float& n);
	matrix& operator*=(const matrix& rhs);
	matrix& operator^=(const float& n);
	vec2	operator*(const vec2& rhs);
	vec3	operator*(const vec3& rhs);
	vec4	operator*(const vec4& rhs);

	friend matrix operator+(matrix lhs, const matrix& rhs) {
		lhs += rhs;
		return lhs;
	}
	friend matrix operator-(matrix lhs, const matrix& rhs) {
		lhs -= rhs;
		return lhs;
	}
	friend matrix operator*(matrix lhs, const float& n) {
		lhs *= n;
		return lhs;
	}
	friend matrix operator*(matrix lhs, const matrix& rhs) {
		lhs *= rhs;
		return lhs;
	}
	friend matrix operator^(matrix lhs, const float& n) {
		lhs ^= n;
		return lhs;
	}

	float* operator[](size_t row);
	const float* operator[](size_t row) const;
	size_t numColumns();
	size_t numRows();
	bool setrow(size_t row, float val);
	std::string to_string();
	float* data();
	gore::matrix inverse();
	// static matrix generation
	static matrix calculateOrtho(uint32_t width, uint32_t height, uint32_t last_width, uint32_t last_height);
	static matrix calculate2DView(float x, float y, float zoom);
	static matrix lookat(gore::vec3 pos, gore::vec3 target, gore::vec3 upVector);
};
}