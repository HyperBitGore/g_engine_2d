#include "../include/g_engine/g_engine_2d.hpp"
#include "../include/g_engine/audio/audio.hpp"
#include "../include/g_engine/util/matrix.hpp"
#include <bitset>
#include <cstdint>
#include <memory>
#include <string>

uint32_t globalWidth = 640;
uint32_t globalHeight = 480;
gore::g_engine_2d eng2("Test Window", 640, 480, USE_VIEW_MATRICE, gore::LogType::BOTH);

std::unique_ptr<gore::trianglerenderer>  triangle_r;
std::unique_ptr<gore::linerenderer>      line_r;
std::unique_ptr<gore::pointrenderer>     point_r;
std::unique_ptr<gore::imagerenderer>     img_r;
std::unique_ptr<gore::grayscalerenderer> gray_r;
std::unique_ptr<gore::fontrenderer>      font_r;
gore::drawpass dr(640, 480, GL_COLOR_ATTACHMENT0);
gore::audioplayer ap(4, gore::LogType::BOTH);
gore::audio aud;
gore::audio s_test;
gore::audio s_test2;
gore::audio s_test3;
gore::audio s_test4;

gore::IMG imgtest;
gore::IMG imgtest_pallete;
gore::IMG imgtest_types;
gore::IMG imgtest_grayalpha;
gore::IMG imgtest_bmp;
gore::IMG atlas_test;
gore::IMG bmptest;
gore::IMG blank_test;
gore::IMG conversion_test;
gore::imageatlas atlas = gore::imageatlas(400, 400, 4, 256);
gore::font f_test;
gore::font open_sans;

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
float zoom = 0.0;

//bezier testing
gore::vec2 bez_m = { 120.0f, 130.0f };

gore::vec2 mos = { 200.0f, 300.0f };

struct invert_vertex{
	float x;
	float y;
	float w;
	float h;
};

class Invert : public gore::renderer<Invert, invert_vertex> {
	protected:
		friend class gore::renderer<Invert, invert_vertex>;
		void shader_setup() override {
			shader.setuniform("screen", width, height);
			shader.setuniform("mtexture", (GLuint)0);
			shader.bind();
			glBindVertexArray(vao);
			glBindBuffer(GL_ARRAY_BUFFER, vertex_buffer);
			glEnableVertexAttribArray(0);
			glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, sizeof(invert_vertex), (void*)0); //position
			glEnableVertexAttribArray(1);
			glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, sizeof(invert_vertex), (void*)(sizeof(float) * 2)); //uv
		}
		Invert(GLsizei width, GLsizei height) : gore::renderer<Invert, invert_vertex> (std::string("resources/invert.vs"), std::string("resources/invert.fs"), width, height, true) {
		}
	public:
	void drawTexture(GLuint texture, gore::vec2 pos, gore::vec2 dim, gore::vec4 uvs){
		vertexs.push_back({pos.x, pos.y, uvs.x, uvs.y}); //first triangle top left vertex
		vertexs.push_back({pos.x + dim.x, pos.y, uvs.x + uvs.z, uvs.y}); //first triangel top right
		vertexs.push_back({pos.x, pos.y + dim.y, uvs.x, uvs.y + uvs.w}); //first triangle tip vertex


		vertexs.push_back({pos.x + dim.x, pos.y + dim.y, uvs.x + uvs.z, uvs.y + uvs.w}); //bottom right
		vertexs.push_back({pos.x, pos.y + dim.y, uvs.x, uvs.y + uvs.w}); //bottom left
		vertexs.push_back({pos.x + dim.x, pos.y, uvs.x + uvs.z, uvs.y}); //top righjt

		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, texture);
		
		shader.bind();
		glBindVertexArray(vao);
		glBindBuffer(GL_ARRAY_BUFFER, vertex_buffer);
		glBufferData(GL_ARRAY_BUFFER, vertexs.size() * sizeof(invert_vertex), &vertexs[0], GL_DYNAMIC_DRAW);

		glDrawArraysExt(GL_TRIANGLES, 0, vertexs.size());
		
		vertexs.clear();
	}
};

std::unique_ptr<Invert> invert = nullptr;
double draw_timer = 0;
bool draw_mode = false;
void renderFunction() {
	dr.clear();
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
		if (eng2.getKeyDown(g_1)) {
			s_cool = 0;
			ap.playFile(s_test, 0);
		}
		else if (eng2.getKeyDown(g_2)) {
			s_cool = 0;
			ap.playFile(s_test2, 0);
		}
		else if (eng2.getKeyDown(g_3)) {
			s_cool = 0;
			ap.playFile(s_test3, 3);
		}
		else if (eng2.getKeyDown(g_4)) {
			s_cool = 0;
			ap.playFile(s_test4, 1);
		}
		
	}

	if (eng2.getMouseLeftDown()) {
		mos = eng2.getMousePos();
		std::cout << mos.x << " : " << mos.y << "\n";
	}
	if (eng2.getKeyDown(g_RightArrow)) {
		bez_m.x += 0.01f;
	}
	else if (eng2.getKeyDown(g_LeftArrow)) {
		bez_m.x -= 0.01f;
	}
	else if (eng2.getKeyDown(g_UpArrow)) {
		if (play_it) {
			ap.pause(1);
			play_it = false;
		}
		else {
			ap.start(1);
			play_it = true;
			
		}
	}
	if (eng2.getKeyDown(g_x)) {
		zoom -= 0.01;
		eng2.updateView(0, 0, zoom);
	} else if (eng2.getKeyDown(g_z)) {
		zoom += 0.01;
		eng2.updateView(0, 0, zoom);
	}
	dr.bind();
	triangle_r->setColor({1.0f, 0.5f, 0.0f, 1.0f});
	triangle_r->drawTriangle({480.0f, 200.0f}, {500.0f, 250.0f}, {520.0f, 200.0f});
	triangle_r->drawTriangle({-480.0f, 200.0f}, {-500.0f, 250.0f}, {-520.0f, 200.0f});
	triangle_r->addTriangle({300.0f, 100.0f}, {320.0f, 120.0f}, {340.0f, 100.0f});
	triangle_r->addTriangle({300.0f, 80.0f}, {320.0f, 100.0f}, {340.0f, 80.0f});
	triangle_r->addTriangle({300.0f, 60.0f}, {320.0f, 80.0f}, {340.0f, 60.0f});
	triangle_r->addTriangle({0.0f, 0.0f}, {20.0f, 20.0f}, {40.0f, 0.0f});
	triangle_r->addTriangle({640.0f, 480.0f}, {620.0f, 460.0f}, {600.0f, 480.0f});
	triangle_r->drawBuffer();
	
	triangle_r->setColor({0.5f, 0.3f, 0.1f, 1.0f});
	triangle_r->drawQuad({pos, 10.0f}, 60.0f, 60.0f);
	point_r->setColor({0.0f, 1.0f, 0.5f, 0.0f});
	point_r->drawPoint({50.0f, 300.0f});
	for (float y = 0.0f; y <= 300.0f; y += 0.1f) {
		point_r->addPoint({70.0f, y});
	}
	point_r->drawBuffer();
	line_r->setColor({0.0f, 0.2f, 1.0f, 1.0f});
	line_r->drawLine({100.0f, 300.0f}, {400.0f, 400.0f});
	triangle_r->setColor({1.0f, 0.2f, 0.5f, 1.0f});
	triangle_r->addCircleFilled({500.0f, 50.0f}, 50.0f);
	triangle_r->drawBuffer();
	line_r->setColor({1.0f, 0.2f, 0.5f, 1.0f});
	line_r->addQuadraticBezier({100.0f, 400.0f}, {250.0f, 350.0f}, {200.0f, 300.0f}, 20);
	line_r->drawBuffer();
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
	float r = float(ang) * (float)M_PI / (float)180.0;
	float r_r = float(r_ang) * (float)M_PI / (float)180.0;
	eng2.enable(GL_BLEND);
	img_r->drawImage(imgtest, {300.0f, 200.0f}, {100.0f, 100.0f});
	img_r->drawImageRotated(atlas_test, {400.0f, 250.0f}, {100.0f, 100.0f}, r);
	img_r->drawImage(blank_test, {300.0f, 330.0f}, {50.0f, 50.0f});
	img_r->drawImageRotated(imgtest, {200.0f, 200.0f}, {100.0f, 100.0f}, r);
	img_r->drawImage(imgtest_pallete, {480.0f, 150.0f}, {100.0f, 100.0f});
	img_r->drawImage(conversion_test, {560.0f, 200.0f}, {100.0f, 100.0f});
	gray_r->drawImage(imgtest_types, {480.0f, 350.0f}, {100.0f, 100.0f});
	gray_r->setWithAlpha(true);
	gray_r->drawImage(imgtest_grayalpha, {480.0f, 280.0f}, {100.0f, 100.0f});
	gray_r->setWithAlpha(false);
	gore::vec4 pos = atlas.getImagePos("atlas_test", true);
	img_r->addImageVertex(atlas.getImg()->tex, {100.0f, 200.0f}, {100.0f, 100.0f}, pos, 0.0f);
	pos = atlas.getImagePos("enem2", true);
	img_r->addImageVertex(atlas.getImg()->tex, {60.0f, 200.0f}, {50.0f, 60.0f}, pos, 0.0f);
	img_r->drawBuffer();
	// img_r.drawImage(imgtest, {0.0f, 0.0f}, {300.0f, 300.0f});
	// img_r.drawImage(imgtest_bmp, {300.0f, 0.0f}, {300.0f, 300.0f});
	eng2.disable(GL_BLEND);
	// img_r.drawImage(atlas.getImg(), {100.0f, 200.0f}, {400.0f, 400.0f});
	dr.unbind();
	if(draw_timer > 1.0f){
		draw_mode = !draw_mode;
		draw_timer = 0.0f;
	}
	if(draw_mode){
		invert->drawTexture(dr.getTexture(), {-1.0f, 1.0f}, {2.0f, -2.0f}, {0.0f, 1.0f, 1.0f, -1.0f});
		// img_r.drawTexture(dr.getTexture(), {0.0f, 0.0f}, {(float)globalWidth, (float)globalHeight}, {0.0f, 1.0f, 1.0f, -1.0f});
	}else{
		img_r->drawTexture(dr.getTexture(), {0.0f, 0.0f}, {(float)globalWidth, (float)globalHeight}, {0.0f, 1.0f, 1.0f, -1.0f});
	}
	eng2.enable(GL_BLEND);
	img_r->drawImage(bmptest, {250.0f, 250.0f}, {(float)200, (float)200});
	//testing font rendering
	gore::fontraster::drawRasterText(&f_test, img_r.get(), "Hello world LOL", 100.0f, 100.0f, 32, eng2.getDPI());
	gore::fontraster::drawRasterText(&f_test, img_r.get(), "abcdefghijklmnopqrstuvwxzy0123456789,;'\"", 50.0f, 500.0f, 48, eng2.getDPI());
	gore::fontraster::drawRasterText(&open_sans, img_r.get(), "The quick brown fox jumps over the lazy dog.", 200.0f, 550.0f, 32, eng2.getDPI());
	gore::fontraster::drawRasterText(&open_sans, img_r.get(), "Hello, fancy seeing you here; Hope you have a nice day!", 200.0f, 700.0f, 32, eng2.getDPI());
	eng2.disable(GL_BLEND);
	font_r->setColor({1.0f, 0.5f, 0.0f, 1.0f});
	font_r->drawText("Hello World qqjj 97 8", &f_test, 100, 30, 24, eng2.getDPI());
	font_r->drawText("Hello, fancy seeing you here; Hope you have a nice day! bb", &open_sans, 200.0f, 650.0f, 32, eng2.getDPI());
	font_r->drawText("o", &open_sans, 200.0f, 850.0f, 128, eng2.getDPI());
	const char test_str[] = {'\x7E', '!', '\x7F', (char)200};
	std::u16string test_str2 = {0xC8, 0x7E, 0x21, 0x10A, 0xFFFF, 0x2DC, 0x144};
	font_r->drawText(test_str2, &f_test, 300.0f, 800.0f, 48, eng2.getDPI());
	gore::fontraster::drawRasterText(&open_sans, img_r.get(), "o~!", 300.0f, 850.0f, 128, eng2.getDPI());
	gore::fontraster::drawRasterText(&open_sans, img_r.get(), "WMabcdefghijklmnopqrstuvwxzy0123456789,;~'\"", 400.0f, 800.0f, 48, eng2.getDPI());
}

int nthBit(int number, int n) {
	return (number >> n) & 1;
}

void windowResize (uint32_t width, uint32_t height) {
	dr.resize(width, height);
	globalWidth = width;
	globalHeight = height;
}


//4278190335
// issue is the image renderer not mapping correctly!
int main() {
	eng2.setWindowResize(windowResize);
	gore::line l11({0.0f, 1.0f}, {10.0f, 8.0f});
	l11.p1.x = 1.0f;
	l11.p2.x = 11.0f;
	gore::matrix matrice(3, 3);
	gore::matrix matrice2(3, 3);
	gore::matrix matrice3(3, 3);
	gore::matrix matrice4(3, 3);
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
	gore::matrix matrice5(3, 3);
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
	ap.playFile("resources/dungeonsynth5.wav", 1);
	for (size_t i = 0; i < 100; i++) {
		//ap.playFile(s_test3, 3);
	}
	bmptest = gore::imageloader::loadBMP("resources/test1.bmp");
	std::cout << gore::imageloader::getPixel(bmptest, 0, 1, 3) << " color at bmp\n";
	imgtest_types = gore::imageloader::loadPNG("resources/Bliss_(Windows_XP)_grayscale16.png");
	imgtest_pallete = gore::imageloader::loadPNG("resources/Bliss_(Windows_XP)_pallette.png");
	imgtest_grayalpha = gore::imageloader::loadPNG("resources/Bliss_(Windows_XP)_grayscalealpha.png");
	imgtest = gore::imageloader::loadPNG("resources/Bliss_(Windows_XP).png");
	imgtest_bmp = gore::imageloader::loadBMP("resources/Bliss_(Windows_XP).bmp");
	atlas_test = gore::imageloader::loadPNG("resources/test.png");
	conversion_test = gore::imageloader::convertIMGRGBA8(imgtest_types);
	gore::imageloader::createTexture(atlas.getImg(), GL_RGBA8, GL_RGBA, GL_UNSIGNED_BYTE);
	atlas.addImage(atlas_test, "atlas_test");
	atlas.addImage(imgtest, "img_test");
	atlas.addImage("resources/enem2_1.png", gore::IMG_TYPE::PNG, "enem2");

	//adding bmp to atlas, don't need width and height
	//atlas.addImage("resources/test1.bmp", IMG_TYPE::BMP, 0, 0, "test1");

	gore::imageloader::updateIMG(atlas.getImg());
	for (int x = 0; x < 100; x++) {
		gore::imageloader::setPixelRaw(imgtest, x, 1, (uint32_t)0xff0000, imgtest->bytes_per_pixel);
	}
	std::cout << gore::imageloader::getPixel(imgtest, 0, 100, 4) << "\n";
	std::cout << gore::imageloader::getPixel(imgtest, 0, 10, 4) << "\n";
	gore::imageloader::updateIMG(imgtest);
	eng2.setRenderFunction(renderFunction);
	f_test = gore::fontloader::loadFont("resources/EnvyCodeR.ttf", 0, 735);
	open_sans =  gore::fontloader::loadFont("resources/OpenSans-Regular.ttf", 32, 127);

	//testing font rasterizing
	gore::fontraster::rasterizeFont(&f_test, 64, eng2.getDPI(), 4278190335, 32, 127);
	gore::fontraster::rasterizeFont(&open_sans, 64, eng2.getDPI(), 4278190335, 32, 127);
	blank_test = gore::imageloader::createBlank(100, 100, 4);
	gore::imageloader::createTexture(blank_test, GL_RGBA8, GL_RGBA, GL_UNSIGNED_BYTE);
	for (int i = 0; i < 100; i++) {
		for(int j = 0; j < 100; j++){
			gore::imageloader::setPixelRaw(blank_test, j, i, 4278190335, 4);
		}
	}
	gore::imageloader::updateIMG(blank_test);


	std::bitset<32> x(10);
	std::cout << x << "\n";
	std::cout << nthBit(10, 2) << "\n";
	std::cout << nthBit(10, 1) << "\n";
	double d = 0;
	double dd = 0;
	eng2.toggleFrameLimitActive();
	eng2.setFrameLimit(600);
	triangle_r = gore::trianglerenderer::create(640, 480);
	line_r     = gore::linerenderer::create(640, 480);
	point_r    = gore::pointrenderer::create(640, 480);
	img_r      = gore::imagerenderer::create(640, 480);
	gray_r     = gore::imagerenderer::create<gore::grayscalerenderer>(640, 480);
	font_r     = gore::fontrenderer::create(640, 480);
	eng2.addRenderer(triangle_r.get(), false, true,  false);
	eng2.addRenderer(line_r.get(),     false, true,  false);
	eng2.addRenderer(point_r.get(),    false, true,  false);
	eng2.addRenderer(img_r.get(),      false, true,  false);
	eng2.addRenderer(gray_r.get(),     false, true,  false);
	eng2.addRenderer(font_r.get(),     false, true, false);
	invert = Invert::create(640, 480);
	while (eng2.updateWindow()) {
		double del = eng2.getDelta();
		eng2.updateInputState();
		d += del;
		dd += del;
		timer += del;
		s_cool += del;
		draw_timer += del;
		std::pair<double, double> frames = eng2.getFrames();
		if(d >= 1.0){
			std::cout << "1 second\n";
			std::cout << "Frames: " << frames.first << ", average time: " << frames.second << "\n";
			d = 0;
		}
		if (dd >= 2.0 && eng2.getKeyDown(g_f)) {
			std::cout << "set full!\n";
			eng2.toggleFullscreen();
			dd = 0;
		} else if (dd >= 2.0 && eng2.getKeyDown(g_c)) {
			std::cout << "capture mouse!\n";
			eng2.toggleMouseCapture(false);
			dd = 0;
		} else if (dd >= 2.0 && eng2.getKeyDown(g_v)) {
			std::cout << "center mouse\n";
			eng2.toggleMouseCapture(true);
			dd = 0;
		} else if (dd >= 2.0 && eng2.getKeyDown(g_b)) {
			std::cout << "toggle mouse hide\n";
			eng2.toggleMouseHide();
			dd = 0;
		}
		if (eng2.getKeyDown(g_Return)) {
			std::cout << "key down\n";
		}
		else if (eng2.getKeyReleased(g_w)) {
			std::cout << "key tapped\n";
		}
	}
	std::cout << "loop exit\n";
	ap.end();
	return 0;
}