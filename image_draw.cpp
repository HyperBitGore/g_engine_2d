#include "g_engine_2d.h"

//2d image drawing functions

void EngineNewGL::bindImg(IMG img) {
	glUseProgram_g(shader_img);
	glBindTextureUnit_g(img->pos, img->tex);
	glUniform1i_g(texuniform_img, img->tex);

	glUseProgram_g(shader_imgr);
	glBindTextureUnit_g(img->pos, img->tex);
	glUniform1i_g(texuniform_imgr, img->pos);
}


//mass draws an image based on buffer_2d
void EngineNewGL::renderImgs(bool blend) {
	if (blend) {
		glEnable(GL_BLEND);
		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	}

	glUseProgram_g(shader_img);
	/*for (size_t i = 0; i < textures.size(); i++) {
		glBindTextureUnit_g(GL_TEXTURE0 + i, textures[i]);
	}
	//we now have weird artifacts in text images
	glUniform1iv_g(texuniform_img, textures.size(), &textures[0]);
	*/
	//glBindTextureUnit_g(img->pos, img->tex);
	//glUniform1i_g(texuniform_img, img->tex);

	glBindVertexArray_g(VAO_Img);

	glBindBuffer_g(GL_ARRAY_BUFFER, img_buffer);
	glBufferData_g(GL_ARRAY_BUFFER, img_vertexs.size() * sizeof(img_vertex), &img_vertexs[0], GL_DYNAMIC_DRAW);

	glDrawArrays_g(GL_TRIANGLES, 0, img_vertexs.size());


	img_vertexs.clear();
	glBindVertexArray_g(0);
	glBindBuffer_g(GL_ARRAY_BUFFER, 0);
	glBindBuffer_g(GL_SHADER_STORAGE_BUFFER, 0);
	glBindTextureUnit_g(GL_TEXTURE0, 0);
	if (blend) {
		glDisable(GL_BLEND);
	}
}


//rotates counter clockwise around top left point, angle is in raidans
//mass draws an image with rotations
void EngineNewGL::renderImgsRotated(bool blend) {
	if (blend) {
		glEnable(GL_BLEND);
		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	}
	glUseProgram_g(shader_imgr);

	//GLuint texUniformLocation = glGetUniformLocation_g(shader_imgr, "image_tex");
	
	GLuint screenu = glGetUniformLocation_g(shader_imgr, "screen"); //for some reason have to keep setting the screen data but it works so idc
	glUniform2f_g(screenu, (float)wind->getWidth(), (float)wind->getHeight());
	glBindVertexArray_g(VAO_Imgr);

	glBindBuffer_g(GL_ARRAY_BUFFER, img_buffer);
	glBufferData_g(GL_ARRAY_BUFFER, img_vertexs.size() * sizeof(img_vertex), &img_vertexs[0], GL_STATIC_DRAW);


	glDrawArrays_g(GL_TRIANGLES, 0, img_vertexs.size());


	img_vertexs.clear();
	glBindVertexArray_g(0);
	glBindBuffer_g(GL_ARRAY_BUFFER, 0);
	glBindTextureUnit_g(0, 0);
	if (blend) {
		glDisable(GL_BLEND);
	}
}


void EngineNewGL::addImageCall(float x, float y, float w, float h, float img_x, float img_y, float img_w, float img_h) {
	float imgx = img_x / img_w;
	float imgy = img_y / img_h;
	float imgw = img_w / img_w;
	float imgh = img_h / img_h;
	//triangle 1
	img_vertexs.push_back({ x, y, 0.0f, imgx, imgy, 0, x, y });
	img_vertexs.push_back({ x + w, y, 0.0f, imgx + imgw, imgy, 0, x, y });
	img_vertexs.push_back({ x, y - h, 0.0f,  imgx, imgy + imgh, 0, x, y });

	//triangle 2
	img_vertexs.push_back({ x + w, y, 0.0f, imgx + imgw, imgy, 0, x, y });
	img_vertexs.push_back({ x + w, y - h, 0.0f, imgx + imgw, imgy + imgh, 0, x, y });
	img_vertexs.push_back({ x, y - h, 0.0f,  imgx, imgy + imgh, 0, x, y });
}

void EngineNewGL::addImageCall(float x, float y, float w, float h) {
	//triangle 1
	img_vertexs.push_back({ x, y, 0.0f, 0.0f, 0.0f, 0, x, y });
	img_vertexs.push_back({ x + w, y, 0.0f, 1.0f, 0.0f, 0, x, y });
	img_vertexs.push_back({ x, y - h, 0.0f,  0.0f, 1.0f, 0, x, y });

	//triangle 2
	img_vertexs.push_back({ x + w, y, 0.0f, 1.0f, 0.0f, 0, x, y });
	img_vertexs.push_back({ x + w, y - h, 0.0f, 1.0f, 1.0f, 0, x, y });
	img_vertexs.push_back({ x, y - h, 0.0f,  0.0f, 1.0f, 0, x, y });

}


//angle in radians
void EngineNewGL::addImageRotatedCall(float x, float y, float w, float h, float ang) {
	//triangle 1
	img_vertexs.push_back({ x, y, 0.0f, 0.0f, 0.0f, ang, x, y });
	img_vertexs.push_back({ x + w, y, 0.0f, 1.0f, 0.0f, ang, x, y });
	img_vertexs.push_back({ x, y - h, 0.0f,  0.0f, 1.0f, ang, x, y });
	//triangle 2
	img_vertexs.push_back({ x + w, y, 0.0f, 1.0f, 0.0f, ang, x, y });
	img_vertexs.push_back({ x + w, y - h, 0.0f, 1.0f, 1.0f, ang, x, y });
	img_vertexs.push_back({ x, y - h, 0.0f,  0.0f, 1.0f, ang, x, y });
}

void EngineNewGL::addImageRotatedCall(IMG img, float x, float y, float w, float h, float ang, float img_x, float img_y, float img_w, float img_h) {
	float imgx = img_x / img->w;
	float imgy = img_y / img->h;
	float imgw = img_w / img->w;
	float imgh = img_h / img->h;
	//triangle 1
	img_vertexs.push_back({ x, y, 0.0f, imgx, imgy, ang, x, y });
	img_vertexs.push_back({ x + w, y, 0.0f, imgx + imgw, imgy, ang, x, y });
	img_vertexs.push_back({ x, y - h, 0.0f,  imgx, imgy + imgh, ang, x, y });

	//triangle 2
	img_vertexs.push_back({ x + w, y, 0.0f, imgx + imgw, imgy, ang, x, y });
	img_vertexs.push_back({ x + w, y - h, 0.0f, imgx + imgw, imgy + imgh, ang, x, y });
	img_vertexs.push_back({ x, y - h, 0.0f,  imgx, imgy + imgh, ang, x, y });
}