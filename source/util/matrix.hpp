#pragma once
#include <iostream>
#include <string>
#include <vector>

//https://en.wikipedia.org/wiki/Matrix_(mathematics)
//https://en.wikipedia.org/wiki/Computational_complexity_of_matrix_multiplication
//https://github.com/g-truc/glm
// -implement perspective, translate, rotate, and scale
//https://www.khanacademy.org/math/linear-algebra/matrix-transformations/lin-trans-examples/v/linear-transformation-examples-scaling-and-reflections
//https://matrixcalc.org/
class Matrix {
private:
	size_t columns;
	size_t rows;
	float* dat;
public:
	Matrix() = delete;
	Matrix(size_t r, size_t c);
	//copy constructor
	Matrix(const Matrix& m);
	~Matrix();

	Matrix& operator=(const Matrix& rhs);
	Matrix& operator+=(const Matrix& rhs);
	Matrix& operator-=(const Matrix& rhs);
	Matrix& operator*=(const float& n);
	Matrix& operator*=(const Matrix& rhs);
	Matrix& operator^=(const float& n);

	friend Matrix operator+(Matrix lhs, const Matrix& rhs) {
		lhs += rhs;
		return lhs;
	}
	friend Matrix operator-(Matrix lhs, const Matrix& rhs) {
		lhs -= rhs;
		return lhs;
	}
	friend Matrix operator*(Matrix lhs, const float& n) {
		lhs *= n;
		return lhs;
	}
	friend Matrix operator*(Matrix lhs, const Matrix& rhs) {
		lhs *= rhs;
		return lhs;
	}
	friend Matrix operator^(Matrix lhs, const float& n) {
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
};