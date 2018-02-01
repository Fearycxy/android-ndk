/*
 * Copyright 2013 The Android Open Source Project
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include "gles3jni.h"
#include <EGL/egl.h>
#include <time.h>

#define STB_IMAGE_IMPLEMENTATION

#include "stb_image.h"

#define STR(s) #s
#define STRV(s) STR(s)

#define POS_ATTRIB 0
#define COLOR_ATTRIB 1
#define SCALEROT_ATTRIB 2
#define OFFSET_ATTRIB 3

static const char VERTEX_SHADER[] =
        "#version 300 es\n"
             "layout (location = 0) in vec3 aPos;\n"
                "layout (location = 1) in vec3 aColor;\n"
                "layout (location = 2) in vec2 aTexCoord;\n"
                "\n"
                "out vec3 ourColor;\n"
                "out vec2 TexCoord;\n"
                "\n"
                "void main()\n"
                "{\n"
                "    gl_Position = vec4(aPos, 1.0);\n"
                "    ourColor = aColor;\n"
                "    TexCoord = aTexCoord;\n"
                "}";

static const char FRAGMENT_SHADER[] =
        "#version 300 es\n"
                "precision mediump float;\n"
                "out vec4 FragColor;\n"
                "\n"
                "in vec3 ourColor;\n"
                "in vec2 TexCoord;\n"
                "\n"
                "uniform sampler2D ourTexture;\n"
                "\n"
                "void main()\n"
                "{\n"
                "FragColor = texture(ourTexture, TexCoord) * vec4(ourColor, 1.0);\n"
                "}";

class RendererES4 : public Renderer {
public:
    RendererES4();

    virtual ~RendererES4();

    bool init();

private:
    enum {
        VB_INSTANCE, VB_SCALEROT, VB_OFFSET, VB_COUNT
    };

    virtual float *mapOffsetBuf();

    virtual void unmapOffsetBuf();

    virtual float *mapTransformBuf();

    virtual void unmapTransformBuf();

    virtual void draw(unsigned int numInstances);

    const EGLContext mEglContext;
    GLuint mProgram;
    GLuint mVB[VB_COUNT];
    GLuint mVBState;
};

Renderer *createES4Renderer() {
    RendererES4 *renderer = new RendererES4;
    if (!renderer->init()) {
        delete renderer;
        return NULL;
    }
    return renderer;
}

RendererES4::RendererES4()
        : mEglContext(eglGetCurrentContext()),
          mProgram(0),
          mVBState(0) {
    for (int i = 0; i < VB_COUNT; i++)
        mVB[i] = 0;
}

unsigned int VBO, VAO, EBO, texture;

bool RendererES4::init() {
    mProgram = createProgram(VERTEX_SHADER, FRAGMENT_SHADER);
    if (!mProgram)
        return false;
    float vertices[] = {
            // positions          // colors           // texture coords
            0.5f, 0.5f, 0.0f, 1.0f, 0.0f, 0.0f, 1.0f, 1.0f, // top right
            0.5f, -0.5f, 0.0f, 0.0f, 1.0f, 0.0f, 1.0f, 0.0f, // bottom right
            -0.5f, -0.5f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f, // bottom left
            -0.5f, 0.5f, 0.0f, 1.0f, 1.0f, 0.0f, 0.0f, 1.0f  // top left
    };
    unsigned int indices[] = {
            0, 1, 3, // first triangle
            1, 2, 3  // second triangle
    };
    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);
    glGenBuffers(1, &EBO);
    glBindVertexArray(VAO);

    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);

    // position attribute
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void *) 0);
    glEnableVertexAttribArray(0);
    // color attribute
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float),
                          (void *) (3 * sizeof(float)));
    glEnableVertexAttribArray(1);
    // texture coord attribute
    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(float),
                          (void *) (6 * sizeof(float)));
    glEnableVertexAttribArray(2);


    // load and create a texture
    // -------------------------
    glGenTextures(1, &texture);
    glBindTexture(GL_TEXTURE_2D,
                  texture); // all upcoming GL_TEXTURE_2D operations now have effect on this texture object
    // set the texture wrapping parameters
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S,
                    GL_REPEAT);    // set texture wrapping to GL_REPEAT (default wrapping method)
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    // set texture filtering parameters
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    // load image, create texture and generate mipmaps
    int width, height, nrChannels;
    // The FileSystem::getPath(...) is part of the GitHub repository so we can find files on any IDE/platform; replace it with your own image path.
    unsigned char *data = stbi_load("/sdcard/wall.jpg", &width, &height, &nrChannels, 0);
    if (data) {
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, data);
        glGenerateMipmap(GL_TEXTURE_2D);
    } else {
        ALOGE("Failed to load texture");
    }
    stbi_image_free(data);
    ALOGV("Using OpenGL ES 3.0 renderer");
    return true;
}

RendererES4::~RendererES4() {
    /* The destructor may be called after the context has already been
     * destroyed, in which case our objects have already been destroyed.
     *
     * If the context exists, it must be current. This only happens when we're
     * cleaning up after a failed init().
     */
/*    if (eglGetCurrentContext() != mEglContext)
        return;
    glDeleteVertexArrays(1, &mVBState);
    glDeleteBuffers(VB_COUNT, mVB);
    glDeleteProgram(mProgram);*/
    glDeleteVertexArrays(1, &VAO);
    glDeleteBuffers(1, &VBO);
    glDeleteBuffers(1, &EBO);
}

float *RendererES4::mapOffsetBuf() {
    glBindBuffer(GL_ARRAY_BUFFER, mVB[VB_OFFSET]);
    return (float *) glMapBufferRange(GL_ARRAY_BUFFER,
                                      0, MAX_INSTANCES * 2 * sizeof(float),
                                      GL_MAP_WRITE_BIT | GL_MAP_INVALIDATE_BUFFER_BIT);
}

void RendererES4::unmapOffsetBuf() {
    glUnmapBuffer(GL_ARRAY_BUFFER);
}

float *RendererES4::mapTransformBuf() {
    glBindBuffer(GL_ARRAY_BUFFER, mVB[VB_SCALEROT]);
    return (float *) glMapBufferRange(GL_ARRAY_BUFFER,
                                      0, MAX_INSTANCES * 4 * sizeof(float),
                                      GL_MAP_WRITE_BIT | GL_MAP_INVALIDATE_BUFFER_BIT);
}

void RendererES4::unmapTransformBuf() {
    glUnmapBuffer(GL_ARRAY_BUFFER);
}

void RendererES4::draw(unsigned int numInstances) {
    glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT);

    // bind Texture
    glBindTexture(GL_TEXTURE_2D, texture);

    // render container
    glUseProgram(mProgram);
    glBindVertexArray(VAO);
    glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
}
