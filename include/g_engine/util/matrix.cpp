#include "matrix.hpp"
#include <cstring>


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
	return (dat + (row * this->columns));
	//return matrice[row];
}
const float* gore::matrix::operator[](size_t row) const {
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