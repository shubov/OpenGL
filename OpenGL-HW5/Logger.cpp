#include "Logger.h"

static const int LOGGER_FILENAME_MAX              = 256;
static char g_LoggerFileName[LOGGER_FILENAME_MAX] = "OpenGL_5.log";

void LoggerCreate(const char *fileName)
{
	FILE *output;

	memset(g_LoggerFileName, 0, LOGGER_FILENAME_MAX);
	strncpy_s(g_LoggerFileName, fileName, LOGGER_FILENAME_MAX - 1);

	fopen_s(&output, g_LoggerFileName, "w");
	if (output != NULL)
		fclose(output);
}

void LoggerDestroy()
{
	//
}

void LoggerWrite(const char *format, ...)
{
	va_list ap;
	FILE    *output;

	fopen_s(&output, g_LoggerFileName, "a+");
	if (output == NULL)
		return;

	va_start(ap, format);
	vfprintf(output, format, ap);
	va_end(ap);

	fclose(output);
}
