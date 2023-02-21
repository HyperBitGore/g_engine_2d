#include "g_engine_2d.h"

Engine eng = Engine(L"Test Window", 640, 480, 250, 300);;
IMG imgtest;
IMG bmptest;

float triangs[] = { 0.0f, 1.0f, 1.0f };

void renderFunction() {
	//eng.drawTriangle(0.5f, 0.5f, 0.2f);
	//eng.drawRectangle(-0.5f, 0.1f, 0.5f, 0.2f);
	//eng.drawRectangle(-0.5f, -0.5f, 0.3f, 0.1f);
	//eng.drawPoint(0.5f, 0.1f);
	//eng.drawCircle(0.1f, -0.6f, 0.1f);
	eng.drawLineBetter(0.1f, 0.2f, 0.6f, 0.4f);
	//eng.drawLine(0.1f, 0.2f, 0.6f, 0.4f);
	eng.drawLineBetter(-0.3f, 0.2f, -0.5f, -0.6f);
	//eng.drawLine(-0.3f, 0.2f, -0.5f, -0.6f);
	eng.renderImg(imgtest, -0.9f, 0.9f, 300, 300);
	eng.renderImgRotated(imgtest, -0.1f, 0.1f, 300, 300, 0);
	eng.renderImg(bmptest, 0.1f, 0.8f, 200, 200);
	//eng.drawGLLine(0.1f, 0.2f, -0.5f, 0.6f);
	//glDrawArrays(GL_TRIANGLES, triangs[0], 1);
}

//4278190335
int main() {
	imgtest = ImageLoader::loadPNG("Bliss_(Windows_XP).png", 300, 241);
	bmptest = ImageLoader::loadBMP("test1.bmp");
	for (int x = 0; x < 100; x++) {
		//eng.setPixel(imgtest, x, 100, 255, 0, 0, 255);
		eng.setPixel(imgtest, x, 100, 4278190335);
	}
	std::cout << eng.getPixel(imgtest, 0, 100) << "\n";
	std::cout << eng.getPixel(imgtest, 0, 10) << "\n";
	eng.updateIMG(imgtest);
	eng.setRenderFunction(renderFunction);
	while (eng.updateWindow()) {
		if (eng.getKeyDown(VK_RETURN)) {
			std::cout << "key down\n";
		}
		else if (eng.getKeyReleased('w')) {
			std::cout << "key tapped\n";
		}
	}
	return 0;
}