#include "g_engine_2d.h"
#include <bitset>

//Engine eng = Engine(L"Test Window", 640, 480, 300, 300);
EngineNewGL eng2("Test Window", 640, 480);
PrimitiveRenderer prim_r(640, 480);
ImageRenderer img_r(640, 480);
FontRenderer font_r(&prim_r);
//DrawPass dr(640, 480, GL_COLOR_ATTACHMENT0);
AudioPlayer ap(4);
Audio aud;
Audio s_test;
Audio s_test2;
Audio s_test3;
Audio s_test4;

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
bool play_it = true;

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
	if (s_cool >= 0.1f) {
		if (eng2.getKeyDown('1')) {
			s_cool = 0;
			ap.playFile(s_test, 0);
		}
		else if (eng2.getKeyDown('2')) {
			s_cool = 0;
			ap.playFile(s_test2, 0);
		}
		else if (eng2.getKeyDown('3')) {
			s_cool = 0;
			ap.playFile(s_test3, 0);
		}
		else if (eng2.getKeyDown('4')) {
			s_cool = 0;
			ap.playFile(s_test4, 0);
		}
		
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
	else if (eng2.getKeyDown(VK_UP)) {
		if (play_it) {
			ap.pause(1);
			play_it = false;
		}
		else {
			ap.start(1);
			play_it = true;
			
		}
	}
	//dr.bind();
	/*prim_r.setColor({1.0f, 0.5f, 0.0f, 0.0f});
	prim_r.drawTriangle({480.0f, 200.0f}, {500.0f, 250.0f}, {520.0f, 200.0f});
	prim_r.drawTriangle({-480.0f, 200.0f}, {-500.0f, 250.0f}, {-520.0f, 200.0f});
	prim_r.addTriangle({300.0f, 100.0f}, {320.0f, 120.0f}, {340.0f, 100.0f});
	prim_r.addTriangle({300.0f, 80.0f}, {320.0f, 100.0f}, {340.0f, 80.0f});
	prim_r.addTriangle({300.0f, 60.0f}, {320.0f, 80.0f}, {340.0f, 60.0f});
	prim_r.addTriangle({0.0f, 0.0f}, {20.0f, 20.0f}, {40.0f, 0.0f});
	prim_r.addTriangle({640.0f, 480.0f}, {620.0f, 460.0f}, {600.0f, 480.0f});
	prim_r.drawBufferTriangle();
	
	prim_r.setColor({0.5f, 0.3f, 0.1f, 0.0f});
	prim_r.drawQuad({pos, 10.0f}, 60.0f, 60.0f);
	prim_r.setColor({0.0f, 1.0f, 0.5f, 0.0f});
	prim_r.drawPoint({50.0f, 300.0f});
	for (float y = 0.0f; y <= 300.0f; y += 0.1f) {
		prim_r.addPoint({70.0f, y});
	}
	prim_r.drawBufferPoint();
	prim_r.setColor({0.0f, 0.2f, 1.0f, 0.0f});
	prim_r.drawLine({100.0f, 300.0f}, {400.0f, 400.0f});
	prim_r.setColor({1.0f, 0.2f, 0.5f, 0.0f});
	prim_r.circle({500.0f, 50.0f}, 50.0f);
	prim_r.quadraticBezier({100.0f, 400.0f}, {250.0f, 350.0f}, {200.0f, 300.0f}, 20);
	prim_r.drawBufferLine();*/
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
	img_r.drawImage(imgtest, 300.0f, 200.0f, 100.0f, 100.0f);

	//testing font rendering
	//eng2.drawRasterText(&f_test, "Hello world LOL", 100.0f, 100.0f, 32);
	//font_r.drawText("Hello World", &f_test, 100, 30, 24);
}

int nthBit(int number, int n) {
	return (number >> n) & 1;
}


//4278190335
int main() {
	Line l11({0.0f, 1.0f}, {10.0f, 8.0f});
	l11.p1.x = 1.0f;
	l11.p2.x = 11.0f;
	Matrix matrice(3, 3);
	Matrix matrice2(3, 3);
	Matrix matrice3(3, 3);
	Matrix matrice4(3, 3);
	matrice4.setrow(0, 2.0f);
	matrice3.setrow(0, 2.0f);
	matrice[1][0] = 1.0f;
	matrice[1][2] = 1.0f;

	matrice3[0][1] = 3.0f;
	matrice3[1][0] = 6.0f;
	matrice3[1][1] = 3.0f;
	matrice3[1][2] = 8.0f;
	matrice3[2][0] = 3.0f;
	matrice3[2][1] = 12.0f;
	matrice3[2][2] = 5.0f;

	for (size_t i = 0; i < matrice3.numRows(); i++) {
		for (size_t j = 0; j < matrice3.numColumns(); j++) {
			std::cout << matrice3[i][j] << " ";
		}
	}
	std::cout << std::endl;
	matrice4[1][0] = 2.0f;
	matrice4[1][1] = 3.0f;
	matrice4[1][2] = 9.0f;
	matrice4[2][0] = 1.0f;
	matrice4[2][1] = 7.0f;
	matrice4[2][2] = 5.0f;
	matrice3 = matrice3 * matrice4;
	std::cout << matrice3.to_string() << "\n";
	matrice2.setrow(1, 2.0f);
	matrice.setrow(0, 1.0f);
	matrice2 += matrice;
	matrice -= matrice2;
	std::cout << matrice2.to_string() << "\n";
	std::cout << matrice.to_string() << "\n";
	Matrix matrice5(3, 3);
	matrice5.setrow(0, 2.0f);
	matrice5.setrow(1, 3.0f);
	matrice5[2][0] = 1.0f;
	matrice5[2][1] = 4.0f;
	matrice5[2][2] = 9.0f;
	matrice5 ^= 2;
	std::cout << matrice5.to_string() << "\n";

	aud = ap.loadWavFile("resources/sound_32.wav");
	s_test = ap.generateSin(300, 200.0f, 44100);
	s_test2 = ap.generateSquare(300, 200.0f, 44100);
	s_test3 = ap.generateTriangle(300, 200.0f, 44100);
	s_test4 = ap.generateSawtooth(300, 200.0f, 44100);
	ap.playFile("resources/dungeonsynth5_24.wav", 1);
	
	//std::vector<uint16_t> foo = {32000, 4052, 4032};
	//std::vector<float> up(foo.begin(), foo.end());

	bmptest = eng2.loadBMP("resources/test1.bmp");
	imgtest = eng2.loadPNG("resources/Bliss_(Windows_XP).png", 300, 241);
	atlas_test = eng2.loadPNG("resources/test.png", 30, 50);
	eng2.createTexture(atlas.getImg(), GL_RGBA8, GL_RGBA);
	atlas.addImage(atlas_test);
	atlas.addImage(imgtest);
	eng2.updateIMG(atlas.getImg());
	for (int x = 0; x < 100; x++) {
		//ImageLoader::setPixel(imgtest, x, 1, 4278190335, 4);
	}
	std::cout << ImageLoader::getPixel(imgtest, 0, 100, 4) << "\n";
	std::cout << ImageLoader::getPixel(imgtest, 0, 10, 4) << "\n";
	eng2.updateIMG(imgtest);
	eng2.setRenderFunction(renderFunction);
	f_test = font_r.loadFont("resources/EnvyCodeR.ttf", 32, 127);

	//testing font rasterizing
	font_r.rasterizeFont(&f_test, 64, 4278190335, {'l'});

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
	ap.end();
	return 0;
}