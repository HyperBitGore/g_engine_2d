#include "g_engine_2d.h"


//Engine eng = Engine(L"Test Window", 640, 480, 300, 300);
EngineNewGL eng2 = EngineNewGL(L"Test Window", 640, 480);
IMG imgtest;
IMG bmptest;

int ang = 0;
int r_ang = 360;
int c = 0;

void renderFunction() {
	eng2.setDrawColor({ 0.2f, 0.5f, 1.0f, 0.0f });
	eng2.drawTriangle(0.5f, 0.3f, 0.6f, 0.5f, 0.8f, 0.3f);
	eng2.drawQuad(-0.4f, 0.3f, 0.2f, 0.2f);
	eng2.setDrawColor({ 1.0f, 0.3f, 0.1f, 0.0f });
	for (float y = 0.4f; y >= -0.1f; y -= 0.01f) {
		eng2.drawPoint(0.3f, y);
	}
	eng2.setDrawColor({ 0.0f, 1.0f, 0.4f, 0.0f });
	eng2.drawLine(-0.4f, -0.4f, 0.1f, -0.8f, 2.0f);
	eng2.drawCircle(0.4f, -0.6f, 0.1f);
	c++;
	if (c >= 50) {
		c = 0;
		ang++;
		r_ang--;
		(r_ang <= 0) ? r_ang = 360 : r_ang; 
		if (ang > 360) {
			ang = 0;
		}
	}
	float r = float(ang) * M_PI / 180.0;
	float r_r = float(r_ang) * M_PI / 180.0;
	//std::cout << r << "\n";
	eng2.addImageCall(0.2f, 0.2f, 0.2f, 0.2f);
	eng2.addImageCall(-0.4f, -0.4f, 0.3f, 0.3f);
	eng2.addImageCall(0.5f, -0.5f, 0.2f, 0.2f);
	eng2.renderImgs(imgtest);
	//eng2.renderImg(imgtest, 0.3f, 0.3f, 0.2f, 0.2f);
	eng2.addImageRotatedCall(-0.2f, -0.1f, 0.2f, 0.2f, r);
	eng2.addImageRotatedCall(-0.5f, 0.2f, 0.2f, 0.2f, r);
	eng2.addImageRotatedCall(0.5f, -0.6f, 0.2f, 0.2f, r_r);
	eng2.renderImgsRotated(imgtest);
	//eng2.renderImgRotated(imgtest, -0.2f, -0.1f, 0.2f, 0.2f, r);
}




//4278190335
int main() {
	ImageLoader img_ld;

	bmptest = img_ld.loadBMP("test1.bmp");
	imgtest = img_ld.loadPNG("Bliss_(Windows_XP).png", 300, 241);
	for (int x = 0; x < 100; x++) {
		ImageLoader::setPixel(imgtest, x, 100, 4278190335);
	}
	std::cout << ImageLoader::getPixel(imgtest, 0, 100) << "\n";
	std::cout << ImageLoader::getPixel(imgtest, 0, 10) << "\n";
	eng2.updateIMG(imgtest);
	eng2.setRenderFunction(renderFunction);
	double d = 0;
	while (eng2.updateWindow()) {
		double del = eng2.getDelta();
		//std::cout << del << "\n";
		d += del;
		std::pair<double, double> frames = eng2.getFrames();
		if(d >= 1.0){
			std::cout << "1 second\n";
			std::cout << "Frames: " << frames.first << ", average time: " << frames.second << "\n";
			d = 0;
		}
		if (eng2.getKeyDown(VK_RETURN)) {
			std::cout << "key down\n";
		}
		else if (eng2.getKeyReleased('w')) {
			std::cout << "key tapped\n";
		}
	}
	return 0;
}