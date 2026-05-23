#include "matrix.hpp"
#include <cstring>
#include <cmath>
#include <stdexcept>


gore::matrix::matrix(size_t r, size_t c, float fill) {
	columns = c;
	rows = r;
	dat = new float[r * c];
	for (size_t i = 0; i < r; i++) {
		for (size_t j = 0; j < c; j++) {
			dat[(i * rows) + j] = fill;
		}
	}
}

gore::matrix::matrix(size_t r, size_t c) {
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
gore::matrix::matrix(const matrix& m) {
	columns = m.columns;
	rows = m.rows;
	dat = new float[rows * columns];
	memcpy(dat, m.dat, rows * columns * sizeof(float));
}
gore::matrix::~matrix() {
	if (dat) {
		delete[] dat;
	}
}
gore::matrix& gore::matrix::operator=(const matrix& rhs) {
	columns = rhs.columns;
	rows = rhs.columns;
	if (dat) {
		delete[] dat;
	}
	dat = new float[rows * columns];
	memcpy(dat, rhs.dat, rows * columns * sizeof(float));
	return *this;
}

gore::matrix& gore::matrix::operator+=(const matrix& rhs) {
	for (size_t i = 0; i < rhs.rows; i++) {
		for (size_t j = 0; j < rhs.columns; j++) {
			(*this)[i][j] += rhs[i][j];
		}
	}
	return *this;
}
gore::matrix& gore::matrix::operator-=(const matrix& rhs) {
	for (size_t i = 0; i < rhs.rows; i++) {
		for (size_t j = 0; j < rhs.columns; j++) {
			(*this)[i][j] += rhs[i][j];
		}
	}
	return *this;
}
gore::matrix& gore::matrix::operator*=(const float& n) {
	for (size_t i = 0; i < rows; i++) {
		for (size_t j = 0; j < columns; j++) {
			(*this)[i][j] *= n;
		}
	}
	return *this;
}
gore::matrix& gore::matrix::operator*=(const matrix& rhs) {
	//this is right
	//looping all rows
	matrix t(rows, columns);
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
gore::vec2	gore::matrix::operator*(const vec2& rhs) {
	// compare the vector width to the column amt in matrix
	if (columns == 2 && rows == 2) {
		gore::vec2 out;
		out.x = ((*this)[0][0] * rhs.x) + ((*this)[0][1] * rhs.y);
		out.y = ((*this)[1][0] * rhs.x) + ((*this)[1][1] * rhs.y);
		return out;
	} 
	throw std::runtime_error("Not enough columns in matrix to multiply against a vec2!");
}
gore::vec3	gore::matrix::operator*(const vec3& rhs) {
	if (columns == 3 && rows == 3) {
		gore::vec3 out;
		out.x = ((*this)[0][0] * rhs.x) + ((*this)[0][1] * rhs.y) + ((*this)[0][2] * rhs.z);
		out.y = ((*this)[1][0] * rhs.x) + ((*this)[1][1] * rhs.y) + ((*this)[1][2] * rhs.z);
		out.z = ((*this)[2][0] * rhs.x) + ((*this)[2][1] * rhs.y) + ((*this)[2][2] * rhs.z);
		return out;
	}
	throw std::runtime_error("Not enough columns in matrix to multiply against a vec3!");
}
gore::vec4	gore::matrix::operator*(const vec4& rhs) {
	if (columns == 4 && rows == 4) {
		gore::vec4 out;
		out.x = ((*this)[0][0] * rhs.x) + ((*this)[0][1] * rhs.y) + ((*this)[0][2] * rhs.z) + ((*this)[0][3] * rhs.w);
		out.y = ((*this)[1][0] * rhs.x) + ((*this)[1][1] * rhs.y) + ((*this)[1][2] * rhs.z) + ((*this)[1][3] * rhs.w);
		out.z = ((*this)[2][0] * rhs.x) + ((*this)[2][1] * rhs.y) + ((*this)[2][2] * rhs.z) + ((*this)[2][3] * rhs.w);
		out.w = ((*this)[3][0] * rhs.x) + ((*this)[3][1] * rhs.y) + ((*this)[3][2] * rhs.z) + ((*this)[3][3] * rhs.w);
		return out;
	}
	throw std::runtime_error("Not enough columns in matrix to multiply against a vec4!");
}

gore::matrix& gore::matrix::operator^=(const float& n) {
	if (n <= 0) {
		//might be right?
		matrix t(this->rows, this->columns);
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


float* gore::matrix::operator[](size_t row) {
	if (row >= this->rows) {
		throw std::runtime_error("Out of bounds in matrix, rows");
	}
	return (dat + (row * this->columns));
}
const float* gore::matrix::operator[](size_t row) const {
	if (row >= this->rows) {
		throw std::runtime_error("Out of bounds in matrix, rows");
	}
	return (dat + (row * this->columns));
}
size_t gore::matrix::numColumns() {
	return columns;
}
size_t gore::matrix::numRows() {
	return rows;
}
bool gore::matrix::setrow(size_t row, float val) {
	if (row >= rows) {
		return false;
	}
	for (size_t i = 0; i < columns; i++) {
		(*this)[row][i] = val;
	}
	return true;
}
std::string gore::matrix::to_string() {
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
float* gore::matrix::data() {
	return dat;
}

gore::matrix gore::matrix::translate(gore::vec3 translation) {
	if (rows == 4 && columns == 4) {
		throw std::runtime_error("Not a 4 by 4 matrix, can't translate!");
	}
	gore::matrix out = *this;
	out[0][3] = translation.x;
	out[1][3] = translation.y;
	out[2][3] = translation.z;

	return out;
}
gore::matrix gore::matrix::translate(gore::vec2 translation) {
	if (rows == 3 && columns == 3) {
		throw std::runtime_error("Not a 3 by 3 matrix, can't translate by a vec2!");
	}
	gore::matrix out = *this;
	out[0][3] = translation.x;
	out[1][3] = translation.y;
	return out;
}

gore::matrix gore::matrix::rotate(gore::vec3 rotate_axis, float radians) {
	if (rows == 4 && columns == 4) {
		throw std::runtime_error("Not a 4 by 4 matrix, can't translate!");
	}
	gore::matrix out = *this;
	return out;
}
gore::matrix gore::matrix::rotate(gore::vec2 rotate_axis, float radians) {
	if (rows == 3 && columns == 3) {
		throw std::runtime_error("Not a 3 by 3 matrix, can't translate by a vec2!");
	}
	gore::matrix out = *this;
	return out;
}
gore::matrix gore::matrix::scale(gore::vec3 scale) {
	if (rows == 4 && columns == 4) {
		throw std::runtime_error("Not a 4 by 4 matrix, can't translate!");
	}
	gore::matrix out = *this;
	return out;
}
gore::matrix gore::matrix::scale(gore::vec2 scale) {
	if (rows == 3 && columns == 3) {
		throw std::runtime_error("Not a 3 by 3 matrix, can't translate by a vec2!");
	}
	gore::matrix out = *this;
	return out;
}

// https://stackoverflow.com/questions/12230312/is-glmortho-actually-wrong
// https://docs.gl/gl3/glOrtho
// height shifting too far up
gore::matrix gore::matrix::calculateOrtho(uint32_t width, uint32_t height, uint32_t last_width, uint32_t last_height) {
	float aspect = (float)width / (float)height;
	float adjustedWidth = (float)width;
	float adjustedHeight = (float)height;
	if (aspect > ((float)last_width / (float)last_height)) {
		adjustedWidth = adjustedHeight * aspect;
	} else {
		adjustedHeight = adjustedWidth / aspect;
	}
	matrix matrice(4, 4);
	matrice[0][0] = 2.0f / adjustedWidth;
	matrice[1][1] = -2.0f / height;
	matrice[2][2] = -2.0f / (1.0f - -1.0f); //zfar

	matrice[2][3] = 0.0f;
	matrice[3][3] = 1.0f;
	matrice[0][3] = -1.0f;
	matrice[1][3] = 1.0f;

	return matrice;
}

gore::matrix gore::matrix::calculate2DView(float x, float y, float zoom) {
	matrix matrice(4, 4);
	// identity
	matrice[0][0] = 1.0f;
	matrice[1][1] = 1.0f;
	matrice[2][2] = 1.0f;
	matrice[3][3] = 1.0f;
	// zoom 
	matrice[0][0] = zoom;
	matrice[1][1] = zoom;
	// translation
	matrice[0][3] = -x * zoom;
	matrice[1][3] = -y * zoom;
	
	return matrice;
}

gore::matrix gore::matrix::inverse() {
	if (this->columns != this->rows) {
		throw std::runtime_error("Can't invert a non-square matrix!");
	}
	matrix augmented = matrix(this->rows, this->rows * 2);
	for (size_t i = 0; i < rows; i++) {
		for (size_t j = 0; j < rows; j++) {
			augmented[i][j] = (*this)[i][j];
			augmented[i][j + rows] = (i == j) ? 1.0f : 0.0f; // identity matrix, 1s in diagonals
		}
	}
	// Gauss-Jordan elimination with partial pivoting
	for (size_t col = 0; col < rows; col++) {
		// Find the row with the largest absolute value in this column (partial pivoting)
		size_t pivotRow = col;
		float maxVal = std::abs(augmented[col][col]);
		for (size_t r = col + 1; r < rows; r++) {
			float val = std::abs(augmented[r][col]);
			if (val > maxVal) {
				maxVal = val;
				pivotRow = r;
			}
		}

		if (maxVal == 0.0f) {
			throw std::runtime_error("Matrix is singular and cannot be inverted!");
		}

		// Swap current row with pivot row
		if (pivotRow != col) {
			for (size_t j = 0; j < rows * 2; j++) {
				float tmp = augmented[col][j];
				augmented[col][j] = augmented[pivotRow][j];
				augmented[pivotRow][j] = tmp;
			}
		}

		// Scale the pivot row so the pivot element becomes 1
		float scale = augmented[col][col];
		for (size_t j = 0; j < rows * 2; j++) {
			augmented[col][j] /= scale;
		}

		// Eliminate this column from all other rows
		for (size_t r = 0; r < rows; r++) {
			if (r == col) continue;
			float factor = augmented[r][col];
			for (size_t j = 0; j < rows * 2; j++) {
				augmented[r][j] -= factor * augmented[col][j];
			}
		}
	}
	gore::matrix inv = gore::matrix(rows, columns);
	for (size_t i = 0; i < rows; i++) {
		for (size_t j = 0; j < rows; j++) {
			inv[i][j] = augmented[i][j + rows];
		}
	}
	return inv;
}


gore::matrix gore::matrix::generateIdentity (uint32_t row, uint32_t cols) {
	matrix mat = matrix(row, cols);
	// put the 1s in the diagonal 
	for (size_t r = 0; r < row; r++) {
		// set the col
		mat[r][r] = 1.0f;
	}
	return mat;
}

gore::matrix gore::matrix::generateModel (gore::vec3 pos, float angle, gore::vec3 rotate_pos) {
	gore::matrix out = gore::matrix::generateIdentity(4, 4);
	out = out.translate(pos);
	// rotate
	// scale
	return out;
}