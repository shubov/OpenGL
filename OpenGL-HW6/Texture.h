#ifndef TEXTURE_H
#define TEXTURE_H

#include "common.h"
#include "OpenGL.h"

// создает текстуру из TGA-файла с изображением
GLuint TextureCreateFromTGA(const char *fileName);

// создание "пустой текстуры"
GLuint TextureCreateEmpty(GLint internalFormat, GLenum format, GLenum type, GLsizei width, GLsizei height);

// создание "пустой" текстуры для хранения depth-данных
GLuint TextureCreateDepth(GLsizei width, GLsizei height);

// создание фейковой текстуры залитой белым цветом
GLuint TextureCreateFake();

// удаление текстуры
void TextureDestroy(GLuint texture);

// установка текстуры в текстурный юнит и шейдер
void TextureSetup(GLuint program, GLint unit, const GLchar *name, GLuint texture);

#endif /* TEXTURE_H */
