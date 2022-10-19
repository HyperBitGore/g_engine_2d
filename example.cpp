#include "g_engine_2d.h"



float triangs[] = { 0.0f, 1.0f, 1.0f };

void renderFunction() {
	glBegin(GL_TRIANGLES);
	glVertex2f(0.0f, 0.0f);
	glVertex2f(0.5f, 1.0f);
	glVertex2f(1.0f, 0.0f);
	glEnd();
	//glDrawArrays(GL_TRIANGLES, triangs[0], 1);
}


int main() {
	Engine eng = Engine(L"Test Window", 640, 480, 250, 300);
	eng.setRenderFunction(renderFunction);
	while (eng.updateWindow());
	return 0;
}