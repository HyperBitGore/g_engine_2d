#include "g_engine_2d.h"


//Engine eng = Engine(L"Test Window", 640, 480, 300, 300);
EngineNewGL eng2 = EngineNewGL(L"Test Window", 640, 480);
IMG imgtest;
IMG bmptest;


void renderFunction() {
	eng2.drawTriangle(0.5f, 0.3f, 0.6f, 0.5f, 0.8f, 0.3f);
	//eng2.drawQuad(-0.4f, 0.3f, 0.2f, 0.2f);
	for (float y = 0.4f; y >= -0.1f; y -= 0.01f) {
		//eng2.drawPoint(0.3f, y);
	}
	//eng2.drawLine(-0.4f, -0.4f, 0.1f, -0.8f, 2.0f);
	//eng2.drawCircle(0.4f, -0.6f, 0.1f);
	eng2.renderImg(imgtest, 0.3f, 0.3f, 0.2f, 0.2f);
}




//4278190335
int main() {
	imgtest = ImageLoader::loadPNG("Bliss_(Windows_XP).png", 300, 241);
	bmptest = ImageLoader::loadBMP("test1.bmp");
	for (int x = 0; x < 100; x++) {
		//ImageLoader::setPixel(imgtest, x, 100, 4278190335);
	}
	//std::cout << ImageLoader::getPixel(imgtest, 0, 100) << "\n";
	//std::cout << ImageLoader::getPixel(imgtest, 0, 10) << "\n";
	//eng2.updateIMG(imgtest);
	eng2.setRenderFunction(renderFunction);
	while (eng2.updateWindow()) {
		if (eng2.getKeyDown(VK_RETURN)) {
			std::cout << "key down\n";
		}
		else if (eng2.getKeyReleased('w')) {
			std::cout << "key tapped\n";
		}
	}
	return 0;
}