#include "matrix.h"


Matrix::Matrix(size_t r, size_t c) {
	columns = c;
	rows = r;
	for (size_t i = 0; i < rows; i++) {
		matrice.push_back({});
	}
	for (size_t i = 0; i < rows; i++) {
		for (size_t j = 0; j < columns; j++) {
			matrice[i].push_back(0.0f);
		}
	}
}
//copy constructor
Matrix::Matrix(const Matrix& m) {
	columns = m.columns;
	rows = m.rows;
	std::copy(m.matrice.begin(), m.matrice.end(), std::back_inserter(matrice));
}
Matrix::~Matrix() {

}
Matrix& Matrix::operator+=(const Matrix& rhs) {
	for (size_t i = 0; i < rhs.rows; i++) {
		for (size_t j = 0; j < rhs.columns; j++) {
			matrice[i][j] += rhs.matrice[i][j];
		}
	}
	return *this;
}
Matrix& Matrix::operator-=(const Matrix& rhs) {
	for (size_t i = 0; i < rhs.rows; i++) {
		for (size_t j = 0; j < rhs.columns; j++) {
			matrice[i][j] -= rhs.matrice[i][j];
		}
	}
	return *this;
}
Matrix& Matrix::operator*=(const float& n) {
	for (size_t i = 0; i < rows; i++) {
		for (size_t j = 0; j < columns; j++) {
			matrice[i][j] *= n;
		}
	}
	return *this;
}
Matrix& Matrix::operator*=(const Matrix& rhs) {
	//this is wrong
	for (size_t i = 0; i < rows; i++) {
		for (size_t j = 0; j < columns; j++) {
			matrice[i][j] *= rhs.matrice[i][j];
		}
	}
	return *this;
}

Matrix& Matrix::operator^=(const float& n) {
	//this is wrong
	for (size_t i = 0; i < rows; i++) {
		for (size_t j = 0; j < columns; j++) {
			matrice[i][j] = powf(matrice[i][j], n);
		}
	}
	return *this;
}


std::vector<float>& Matrix::operator[](size_t row) {
	return matrice[row];
}
const std::vector<float>& Matrix::operator[](size_t row) const {
	return matrice[row];
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
		matrice[row][i] = val;
	}
	return true;
}
std::string Matrix::to_string() {
	std::string ret = "";
	for (size_t i = 0; i < rows; i++) {
		ret += "row " + std::to_string(i) + ":";
		for (size_t j = 0; j < columns; j++) {
			ret += std::to_string(matrice[i][j]) + ",";
		}
		ret += ";";
	}
	return ret;
}
std::vector<std::vector<float>>& Matrix::data() {
	return matrice;
}