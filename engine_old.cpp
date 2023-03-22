#include "g_engine_2d.h"



bool Engine::updateWindow() {
	UpdateWindow(wind->getHwnd());
	if (!wind->ProcessMessage()) {
		std::cout << "Closing window\n";
		delete wind;
		return false;
	}
	glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	renderFund();
	SwapBuffers(dc_w);
	return true;
}


void Engine::drawTriangle(float x, float y, float size) {
	glBegin(GL_TRIANGLES);
	glVertex2f(x - size, y - size);
	glVertex2f(x, y);
	glVertex2f(x + size, y - size);
	glEnd();
}


void Engine::drawCircle(float x, float y, float r) {
	float x1, y1;
	glBegin(GL_POINTS);
	for (float ang = 0; ang < 360; ang += 1.0f) {
		x1 = float(r * cos(ang * M_PI / 180) + x);
		y1 = float(r * sin(ang * M_PI / 180) + y);
		glVertex2f(x1, y1);
	}
	glEnd();
}


void Engine::drawPoint(float x, float y) {
	glBegin(GL_POINTS);
	glVertex2f(x, y);
	glEnd();
}

void Engine::drawRectangle(float x, float y, float w, float h) {
	glBegin(GL_QUADS);
	glVertex2f(x, y);
	glVertex2f(x, y - h);
	glVertex2f(x + w, y - h);
	glVertex2f(x + w, y);
	glEnd();
}

void Engine::drawLine(float x1, float y1, float x2, float y2) {
	glBegin(GL_POINTS);
	float dx = x2 - x1;
	float dy = y2 - y1;
	//draw points while we travel with angle to second point
	if (x1 > x2) {
		for (float x = x1; x > x2; x += -0.001f) {
			float y = y1 + dy * (x - x1) / dx;
			glVertex2f(x, y);
		}
	}
	else {
		for (float x = x1; x < x2; x += 0.001f) {
			float y = y1 + dy * (x - x1) / dx;
			glVertex2f(x, y);
		}
	}
	glEnd();
}

void Engine::drawLineBetter(float x1, float y1, float x2, float y2) {
	glBegin(GL_POINTS);
	float dx = std::abs(x2 - x1);
	float dy = std::abs(y2 - y1);
	if (dx > dy) {
		if (x1 > x2) {
			float t = x1;
			x1 = x2;
			x2 = t;
			t = y1;
			y1 = y2;
			y2 = t;
		}
		float a = ((y2 - y1) / (x2 - x1)) / wind->getHeight();
		float d = y1;
		float inc = 1.0f / wind->getWidth();
		for (float x = x1; x <= x2; x += inc) {
			glVertex2f(x, d);
			d = d + a;
		}

	}
	else {
		if (y1 > y2) {
			float t = x1;
			x1 = x2;
			x2 = t;
			t = y1;
			y1 = y2;
			y2 = t;
		}
		float a = ((x2 - x1) / (y2 - y1)) / wind->getWidth();
		float d = x1;
		float inc = 1.0f / wind->getHeight();
		for (float y = y1; y <= y2; y += inc) {
			glVertex2f(d, y);
			d = d + a;
		}
	}
	glEnd();
}


void Engine::renderImg(IMG img, float x, float y, int w, int h) {
	float r_w = (float(w) / wind->getWidth());
	float r_h = (float(h) / wind->getHeight());
	glBindTexture(GL_TEXTURE_2D, (GLuint)img->tex);
	glBegin(GL_QUADS);
	glTexCoord2f(0, 0);
	glVertex2f(x, y);
	glTexCoord2f(1, 0);
	glVertex2f(x + r_w, y);
	glTexCoord2f(1, 1);
	glVertex2f(x + r_w, y - r_h);
	glTexCoord2f(0, 1);
	glVertex2f(x, y - r_h);
	glEnd();
	glBindTexture(GL_TEXTURE_2D, 0);
}

void Engine::renderImgRotated(IMG img, float x, float y, int w, int h, int ang) {
	float r_ang = float(ang) * float(M_PI / 180.0f);
	float r_cos = cosf(r_ang);
	float r_sin = sinf(r_ang);
	float r_x = (x * r_cos) - (y * r_sin);
	float r_y = (x * r_sin) + (y * r_cos);
	float r_w = (float(w) / wind->getWidth());
	float r_h = (float(h) / wind->getHeight());
	glBindTexture(GL_TEXTURE_2D, (GLuint)img->tex);
	glBegin(GL_QUADS);
	glTexCoord2f(0, 0);
	glVertex2f(((x * r_cos) - (y * r_sin)), ((x * r_sin) + (y * r_cos)));
	glTexCoord2f(1, 0);
	glVertex2f(((x + r_w) * r_cos) - (y * r_sin), (((x + r_w) * r_sin) + (y * r_cos)));
	glTexCoord2f(1, 1);
	glVertex2f(((x + r_w) * r_cos) - ((y - r_h) * r_sin), (((x + r_w) * r_sin) + ((y - r_h) * r_cos)));
	glTexCoord2f(0, 1);
	glVertex2f(((x * r_cos) - ((y - r_h) * r_sin)), ((x * r_sin) + ((y - r_h) * r_cos)));
	glEnd();
	glBindTexture(GL_TEXTURE_2D, 0);
}