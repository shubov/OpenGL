﻿#include "Camera.h"

void CameraCreate(Camera &camera, float x, float y, float z)
{
	// зададим позицию камеры и единичную матрицу проекции
	camera.position   = vec3(x, y, z);
	camera.rotation   = vec3_zero;
	camera.projection = mat4_identity;
}

void CameraLookAt(Camera &camera, const vec3 &position, const vec3 &center, const vec3 &up)
{
	camera.rotation = GLToEuler(GLLookAt(position, center, up));
	camera.position = position;
}

void CameraPerspective(Camera &camera, float fov, float aspect, float zNear, float zFar)
{
	// расчет перспективной матрицы проекции
	camera.projection = GLPerspective(fov, aspect, zNear, zFar);
}

void CameraOrtho(Camera &camera, float left, float right,
	float bottom, float top, float zNear, float zFar)
{
	// расчет ортографической матрицы проекции
	camera.projection = GLOrthographic(left, right, bottom, top, zNear, zFar);
}

void CameraRotate(Camera &camera, float x, float y, float z)
{
	// увеличение углов вращения камеры
	camera.rotation += vec3(x, y, z);
}

void CameraMove(Camera &camera, float x, float y, float z)
{
	// сначала надо перевести вектор направления в локальные координаты камеры
	// для этого надо инвертировать матрицу вращения камеры и умножить ее на вектор
	// однако для матрицы вращения транспонирование дает тот же результат что инвертирование
	vec3 move = transpose(mat3(GLFromEuler(camera.rotation))) * vec3(x, y, z);

	camera.position += move;
}

void CameraSetup(GLuint program, const Camera &camera, const mat4 &model)
{
	// расчитаем необходимые матрицы
	mat4 view           = GLFromEuler(camera.rotation) * GLTranslation(-camera.position);
	mat4 viewProjection = camera.projection * view;
	mat3 normal         = transpose(mat3(inverse(model)));

	mat4 modelViewProjection = viewProjection * model;

	// передаем матрицы в шейдерную программу как uniform структуру
	glUniformMatrix4fv(glGetUniformLocation(program, "transform.model"),          1, GL_TRUE, model.m);
	glUniformMatrix4fv(glGetUniformLocation(program, "transform.viewProjection"), 1, GL_TRUE, viewProjection.m);
	glUniformMatrix3fv(glGetUniformLocation(program, "transform.normal"),         1, GL_TRUE, normal.m);

	glUniformMatrix4fv(glGetUniformLocation(program, "transform.modelViewProjection"),
		1, GL_TRUE, modelViewProjection.m);

	// передаем позицию наблюдателя (камеры) в шейдерную программу
	glUniform3fv(glGetUniformLocation(program, "transform.viewPosition"), 1, camera.position.v);
}

void CameraSetupLightMatrix(GLuint program, const Camera &camera)
{
	// матрица сдвига текстурных координат для перехода из СК [-1, 1] в [0, 1]
	static const mat4 bias(
		0.5f, 0.0f, 0.0f, 0.5f,
		0.0f, 0.5f, 0.0f, 0.5f,
		0.0f, 0.0f, 0.5f, 0.5f,
		0.0f, 0.0f, 0.0f, 1.0f
	);

	// расчитаем необходимые матрицы
	mat4 view           = GLFromEuler(camera.rotation) * GLTranslation(-camera.position);
	mat4 viewProjection = bias * camera.projection * view;

	// передадим матрицу источника освещения в шейдерную программу
	glUniformMatrix4fv(glGetUniformLocation(program, "transform.light"), 1, GL_TRUE, viewProjection.m);
}
