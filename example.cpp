#include "g_engine_2d.h"
#include <bitset>

//Engine eng = Engine(L"Test Window", 640, 480, 300, 300);
EngineNewGL eng2(L"Test Window", 640, 480);
AudioPlayer ap;
Audio aud;

IMG imgtest;
IMG atlas_test;
IMG bmptest;
IMG blank_test;
ImageAtlas atlas = ImageAtlas(400, 400, 4);
Font f_test;

int ang = 0;
int r_ang = 360;
int c = 0;
float pos = 0;
float posy = 0;
bool dir = false;
bool dir2 = false;
double timer = 0;
double s_cool = 0;

//bezier testing
vec2 bez_m = { 120.0f, 130.0f };

vec2 mos = { 200.0f, 300.0f };

void renderFunction() {
	if (timer >= 0.01f) {
		if (pos <= 0.0f) {
			dir = false;
		}
		else if (pos >= 640.0f) {
			dir = true;
		}
		if (posy <= 0.0f) {
			dir2 = false;
		}
		else if (posy >= 480.0f) {
			dir2 = true;
		}
		(!dir) ? pos += 1.0f : pos -= 1.0f;
		(!dir2) ? posy += 1.0f : posy -= 1.0f;
		timer = 0;
		
	}
	if (s_cool >= 0.8f) {
		ap.playFile(aud);
		s_cool = 0;
	}

	if (eng2.getMouseLeftDown()) {
		mos = eng2.getMousePos();
		std::cout << mos.x << " : " << mos.y << "\n";
	}
	if (eng2.getKeyDown(VK_RIGHT)) {
		bez_m.x += 0.01f;
	}
	else if (eng2.getKeyDown(VK_LEFT)) {
		bez_m.x -= 0.01f;
	}
	eng2.setDrawColor({ 0.2f, 0.5f, 1.0f, 0.0f });
	eng2.drawTriangle(100.0f, 120.0f, 130.0f, 100.0f, 150.0f, 120.0f);
	eng2.drawQuad(pos, 10.0f, 60.0f, 60.0f);
	eng2.drawQuad(0.0f, posy, 50.0f, 50.0f);
	eng2.drawQuad(600.0f, 300.0f, 50.0f, 50.0f);
	eng2.drawQuad(640.0f, 400.0f, 50.0f, 50.0f);
	eng2.setDrawColor({ 1.0f, 0.3f, 0.1f, 0.0f });
	for (float y = 100.0f; y <= 300.0f; y += 1.00f) {
		//eng2.addLinePoints({ 1.0f, y }, { 0.5f, y + 1.0f });
		eng2.add2DPoint(50.0f, y);
		//eng2.drawPoint(50.0f, y);
	}
	//eng2.drawLines(0.2f);
	eng2.drawPoints();
	eng2.setDrawColor({ 0.0f, 1.0f, 0.4f, 0.0f });
	eng2.drawLine(350.0f, 200.0f, 450.0f, 420.0f, 2.0f);
	//eng2.drawCircle(400.0f, 250.0f, 50.0f);
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
	eng2.addImageCall( 450.0f, 300.0f, 50.0f, 50.0f);
	eng2.addImageCall( 250.0f, 250.0f, 80.0f, 80.0f);
	eng2.addImageCall( 360.0f, 250.0f, 50.0f, 50.0f);
	eng2.renderImgs(imgtest, false);
	//eng2.renderImg(imgtest, 100.0f, 100.0f, 40.0f, 40.0f);
	eng2.addImageRotatedCall(150.0f, 80.0f, 50.0f, 50.0f, r);
	eng2.addImageRotatedCall(325.0f, 160.0f, 50.0f, 50.0f, r);
	eng2.addImageRotatedCall(350.0f, 350.0f, 50.0f, 50.0f, r_r);
	eng2.renderImgsRotated(imgtest, false);
	//eng2.renderImgRotated(imgtest, -0.2f, -0.1f, 0.2f, 0.2f, r);
	
	eng2.addImageCall(200.0f, 300.0f, 50.0f, 50.0f, atlas.getImagePos(atlas_test).x, atlas.getImagePos(atlas_test).y, 30, 50);
	eng2.addImageCall(300.0f, 300.0f, 50.0f, 50.0f, atlas.getImagePos(imgtest).x, atlas.getImagePos(imgtest).y, 300, 241);
	eng2.renderImgs(atlas.getImg(), false);

	//eng2.renderImg(f_test.glyphs[17].data, 100.0f, 100.0f, 64, 64);
	//eng2.renderImg(blank_test, 100.0f, 100.0f, 64, 64);

	//testing beziers
	//eng2.quadraticBezier({ 300.0f, 100.0f }, { 350.0f, 150.0f }, { 400.0f, 100.0f }, 20);
	//eng2.cubicBezier({ 300.0f, 400.0f }, { 325.0f, 425.0f }, { 350.0f, 425.0f }, { 375.0f, 400.0f }, 20);
	eng2.quadraticBezier({ 50.0f, 80.0f }, bez_m, { 220.0f, 250.0f }, 20);

	//testing font rendering
	eng2.drawRasterText(&f_test, "Hello world LOL", 100.0f, 100.0f, 32);
	eng2.drawText("Hello World", &f_test, 100, 30, 24);
	eng2.setDrawColor({ 1.0f, 0.1f, 0.5f, 1.0f });
	eng2.drawLinePoints({ 100.0f, 200.0f }, mos);
}

int nthBit(int number, int n) {
	return (number >> n) & 1;
}


//4278190335
int main() {
	//ImageLoader img_ld;
	Audio ad2 = ap.loadWavFile("dungeonsynth5.wav");
	aud = ap.loadWavFile("sound.wav");
	//ap.playFile(ad2);
	//ap.start();
	


	bmptest = eng2.loadBMP("test1.bmp");
	imgtest = eng2.loadPNG("Bliss_(Windows_XP).png", 300, 241);
	atlas_test = eng2.loadPNG("test.png", 30, 50);
	eng2.createTexture(atlas.getImg(), GL_RGBA8, GL_RGBA);
	atlas.addImage(atlas_test);
	atlas.addImage(imgtest);
	eng2.updateIMG(atlas.getImg());
	for (int x = 0; x < 100; x++) {
		ImageLoader::setPixel(imgtest, x, 1, 4278190335, 4);
	}
	std::cout << ImageLoader::getPixel(imgtest, 0, 100, 4) << "\n";
	std::cout << ImageLoader::getPixel(imgtest, 0, 10, 4) << "\n";
	eng2.updateIMG(imgtest);
	eng2.setRenderFunction(renderFunction);
	f_test = eng2.loadFont("EnvyCodeR.ttf", 32, 127);

	//testing font rasterizing
	eng2.rasterizeFont(&f_test, 64, 4278190335, {'l'});

	blank_test = ImageLoader::generateBlankIMG(100, 100, 4);
	for (int i = 0; i < 100; i++) {
		ImageLoader::setPixel(blank_test, i, 50, 4278190335, 4);
	}
	eng2.createTexture(blank_test, GL_RGBA8, GL_RGBA);


	std::bitset<32> x(10);
	std::cout << x << "\n";
	std::cout << nthBit(10, 2) << "\n";
	std::cout << nthBit(10, 1) << "\n";
	double d = 0;
	while (eng2.updateWindow()) {
		double del = eng2.getDelta();
		//std::cout << del << "\n";
		d += del;
		timer += del;
		s_cool += del;
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
	ap.pause();
	ap.clear();
	ap.end();
	return 0;
}