#pragma once

#ifdef _WIN32
#include "glew.h"
#else
#include "GL/glew.h"
#endif

// Shader API
GLuint read_program(const char *filename, const char *definitions = "");
int release_program(const GLuint program);
int reload_program(const GLuint program, const char* sourceDir, const char *filename, const char *definitions = "");
int program_format_errors(const GLuint program, std::string& errors);
int program_print_errors(const GLuint program);

