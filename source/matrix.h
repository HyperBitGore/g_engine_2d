#pragma once
#include <iostream>
#include <string>
#include <vector>

//https://en.wikipedia.org/wiki/Matrix_(mathematics)
//https://en.wikipedia.org/wiki/Computational_complexity_of_matrix_multiplication
// convert to not using vectors
//https://github.com/g-truc/glm
// -implement perspective, translate, rotate, and scale
//https://matrixcalc.org/
class Matrix {
private:
	size_t columns;
	size_t rows;
	std::vector<std::vector<float>> matrice;
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

	std::vector<float>& operator[](size_t row);
	const std::vector<float>& operator[](size_t row) const;
	size_t numColumns();
	size_t numRows();
	bool setrow(size_t row, float val);
	std::string to_string();
	std::vector<std::vector<float>>& data();
};