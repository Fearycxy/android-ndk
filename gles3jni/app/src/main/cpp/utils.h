//
// Created by 曹祥昱 on 2018/2/1.
//

#ifndef GLES3JNI_UTILS_H
#define GLES3JNI_UTILS_H
#include <sys/time.h>
#include <string>
#include <EGL/egl.h>
#include "gles3jni.h"
#include "glm/glm.hpp"
#include "glm/gtc/matrix_transform.hpp"
#include "glm/gtc/type_ptr.hpp"
long getCurrentTime();

void setBool(const GLuint ID,const std::string &name, bool value) ;
// ------------------------------------------------------------------------
void setInt(const GLuint ID,const std::string &name, int value) ;
// ------------------------------------------------------------------------
void setFloat(const GLuint ID,const std::string &name, float value) ;
// ------------------------------------------------------------------------
void setVec2(const GLuint ID,const std::string &name, const glm::vec2 &value) ;
void setVec2(const GLuint ID,const std::string &name, float x, float y) ;
// ------------------------------------------------------------------------
void setVec3(const GLuint ID,const std::string &name, const glm::vec3 &value) ;
void setVec3(const GLuint ID,const std::string &name, float x, float y, float z) ;
// ------------------------------------------------------------------------
void setVec4(const GLuint ID,const std::string &name, const glm::vec4 &value)  ;

void setVec4(const GLuint ID,const std::string &name, float x, float y, float z, float w);
// ------------------------------------------------------------------------
void setMat2(const GLuint ID,const std::string &name, const glm::mat2 &mat) ;
// ------------------------------------------------------------------------
void setMat3(const GLuint ID,const std::string &name, const glm::mat3 &mat) ;
// ------------------------------------------------------------------------
void setMat4(const GLuint ID,const std::string &name, const glm::mat4 &mat)  ;
#endif //GLES3JNI_UTILS_H
