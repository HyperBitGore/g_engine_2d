#include "matrix.h"


Matrix::Matrix(size_t r, size_t c) {
	columns = c;
	rows = r;
	dat = new float[r * c];
	for (size_t i = 0; i < r; i++) {
		for (size_t j = 0; j < c; j++) {
			dat[(i * rows) + j] = 0.0f;
		}
	}
}
//copy constructor
Matrix::Matrix(const Matrix& m) {
	columns = m.columns;
	rows = m.rows;
	dat = new float[rows * columns];
	std::memcpy(dat, m.dat, rows * columns * sizeof(float));
}
Matrix::~Matrix() {
	if (dat) {
		delete[] dat;
	}
}
Matrix& Matrix::operator=(const Matrix& rhs) {
	columns = rhs.columns;
	rows = rhs.columns;
	if (dat) {
		delete[] dat;
	}
	dat = new float[rows * columns];
	std::memcpy(dat, rhs.dat, rows * columns * sizeof(float));
	return *this;
}

Matrix& Matrix::operator+=(const Matrix& rhs) {
	for (size_t i = 0; i < rhs.rows; i++) {
		for (size_t j = 0; j < rhs.columns; j++) {
			(*this)[i][j] += rhs[i][j];
		}
	}
	return *this;
}
Matrix& Matrix::operator-=(const Matrix& rhs) {
	for (size_t i = 0; i < rhs.rows; i++) {
		for (size_t j = 0; j < rhs.columns; j++) {
			(*this)[i][j] += rhs[i][j];
		}
	}
	return *this;
}
Matrix& Matrix::operator*=(const float& n) {
	for (size_t i = 0; i < rows; i++) {
		for (size_t j = 0; j < columns; j++) {
			(*this)[i][j] *= n;
		}
	}
	return *this;
}
Matrix& Matrix::operator*=(const Matrix& rhs) {
	//this is right
	//looping all rows
	Matrix t(rows, columns);
	for (size_t i = 0; i < rows; i++) {
		//outer loop setting the actual element value in rows
		for (size_t p = 0; p < columns; p++) {
			//adding up entire and row and opposite column
			float out = 0;
			for (size_t k = 0; k < columns; k++) {
				out += (*this)[i][k] * rhs[k][p];
			}
			t[i][p] = out;
		}
	}
	*this = t;
	return *this;
}

Matrix& Matrix::operator^=(const float& n) {
	if (n <= 0) {
		//might be right?
		Matrix t(this->rows, this->columns);
		for (size_t i = 0, k = 0; i < rows; i++, k++) {
			t[i][k] = 1.0f;
		}
		*this = t;
		return *this;
	}
	//this is right
	for (size_t i = 2; i <= n; i++) {
		*this = *this * *this;
	}
	return *this;
}


float* Matrix::operator[](size_t row) {
	return (dat + (row * this->columns));
	//return matrice[row];
}
const float* Matrix::operator[](size_t row) const {
	return (dat + (row * this->columns));
}
size_t Matrix::numColumns() {
	return columns;
}
size_t Matrix::numRows() {
	return rows;
}
bool Matrix::setrow(size_t row, float val) {
	if (row >= rows) {
		return false;
	}
	for (size_t i = 0; i < columns; i++) {
		(*this)[row][i] = val;
	}
	return true;
}
std::string Matrix::to_string() {
	std::string ret = "";

	for (size_t i = 0; i < rows; i++) {
		ret += "row " + std::to_string(i) + ":";
		for (size_t j = 0; j < columns; j++) {
			ret += std::to_string((*this)[i][j]) + ",";
		}
		ret += ";";
	}
	return ret;
}
float* Matrix::data() {
	return dat;
}