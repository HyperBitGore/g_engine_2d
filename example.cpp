#include "g_engine_2d.h"

Engine eng = Engine(L"Test Window", 640, 480, 250, 300);;

float triangs[] = { 0.0f, 1.0f, 1.0f };

void renderFunction() {
	//eng.drawTriangle(0.5f, 0.5f, 0.2f);
	//eng.drawRectangle(-0.5f, 0.1f, 0.5f, 0.2f);
	//eng.drawRectangle(-0.5f, -0.5f, 0.3f, 0.1f);
	//eng.drawPoint(0.5f, 0.1f);
	//eng.drawCircle(0.1f, -0.6f, 0.1f);
	eng.drawLine(0.1f, 0.2f, 0.6f, 0.4f);
	eng.drawLine(-0.3f, 0.2f, -0.5f, -0.6f);
	//glDrawArrays(GL_TRIANGLES, triangs[0], 1);
}


int main() {
	eng.setRenderFunction(renderFunction);
	while (eng.updateWindow());
	return 0;
}