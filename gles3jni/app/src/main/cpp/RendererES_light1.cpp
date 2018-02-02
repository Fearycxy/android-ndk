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

#include "camera.h"


#define STB_IMAGE_IMPLEMENTATION

#define STR(s) #s
#define STRV(s) STR(s)

#define POS_ATTRIB 0
#define COLOR_ATTRIB 1
#define SCALEROT_ATTRIB 2
#define OFFSET_ATTRIB 3

static const char COLOR_VERTEX_SHADER[] =
        "#version 300 es\n"
                "layout (location = 0) in vec3 aPos;\n"
                "\n"
                "uniform mat4 model;\n"
                "uniform mat4 view;\n"
                "uniform mat4 projection;\n"
                "\n"
                "void main()\n"
                "{\n"
                "\tgl_Position = projection * view * model * vec4(aPos, 1.0);\n"
                "}";

static const char COLOR_FRAGMENT_SHADER[] =
        "#version 300 es\n"
                "precision mediump float;\n"
                "out vec4 FragColor;\n"
                "  \n"
                "uniform vec3 objectColor;\n"
                "uniform vec3 lightColor;\n"
                "\n"
                "void main()\n"
                "{\n"
                "    FragColor = vec4(lightColor * objectColor, 1.0);\n"
                "}";

static const char LAMP_VERTEX_SHADER[] =
        "#version 300 es\n"
                "layout (location = 0) in vec3 aPos;\n"
                "\n"
                "uniform mat4 model;\n"
                "uniform mat4 view;\n"
                "uniform mat4 projection;\n"
                "\n"
                "void main()\n"
                "{\n"
                "\tgl_Position = projection * view * model * vec4(aPos, 1.0);\n"
                "}";

static const char LAMP_FRAGMENT_SHADER[] =
        "#version 300 es\n"
                "precision mediump float;\n"
                "out vec4 FragColor;\n"
                "\n"
                "void main()\n"
                "{\n"
                "    FragColor = vec4(1.0); // set alle 4 vector values to 1.0\n"
                "}";

class RendererES_light1 : public Renderer {
public:
    RendererES_light1();

    virtual ~RendererES_light1();

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
    GLuint lightingShader;
    GLuint lampShader;
    GLuint mVB[VB_COUNT];
    GLuint mVBState;
    float lastFrame;
};

// settings
const unsigned int SCR_WIDTH = 800;
const unsigned int SCR_HEIGHT = 600;

// camera
Camera camera(glm::vec3(0.0f, 0.0f, 3.0f));
// lighting
glm::vec3 lightPos(1.2f, 1.0f, 2.0f);

Renderer *createES4Renderer() {
    RendererES_light1 *renderer = new RendererES_light1;
    if (!renderer->init()) {
        delete renderer;
        return NULL;
    }
    return renderer;
}

RendererES_light1::RendererES_light1()
        : mEglContext(eglGetCurrentContext()),
          lightingShader(0),
          lampShader(0),
          lastFrame(0),
          mVBState(0) {
    for (int i = 0; i < VB_COUNT; i++)
        mVB[i] = 0;
}

unsigned int VBO, cubeVAO, lightVAO, texture;

bool RendererES_light1::init() {
    lightingShader = createProgram(COLOR_VERTEX_SHADER, COLOR_FRAGMENT_SHADER);
    lampShader = createProgram(LAMP_VERTEX_SHADER, LAMP_FRAGMENT_SHADER);
    if (!lightingShader) {
        ALOGE("!lightingShader");
        return false;
    }
    if (!lampShader) {
        ALOGE("!lampShader");
        return false;
    }
    // set up vertex data (and buffer(s)) and configure vertex attributes
    // ------------------------------------------------------------------
    float vertices[] = {
            -0.5f, -0.5f, -0.5f,
            0.5f, -0.5f, -0.5f,
            0.5f, 0.5f, -0.5f,
            0.5f, 0.5f, -0.5f,
            -0.5f, 0.5f, -0.5f,
            -0.5f, -0.5f, -0.5f,

            -0.5f, -0.5f, 0.5f,
            0.5f, -0.5f, 0.5f,
            0.5f, 0.5f, 0.5f,
            0.5f, 0.5f, 0.5f,
            -0.5f, 0.5f, 0.5f,
            -0.5f, -0.5f, 0.5f,

            -0.5f, 0.5f, 0.5f,
            -0.5f, 0.5f, -0.5f,
            -0.5f, -0.5f, -0.5f,
            -0.5f, -0.5f, -0.5f,
            -0.5f, -0.5f, 0.5f,
            -0.5f, 0.5f, 0.5f,

            0.5f, 0.5f, 0.5f,
            0.5f, 0.5f, -0.5f,
            0.5f, -0.5f, -0.5f,
            0.5f, -0.5f, -0.5f,
            0.5f, -0.5f, 0.5f,
            0.5f, 0.5f, 0.5f,

            -0.5f, -0.5f, -0.5f,
            0.5f, -0.5f, -0.5f,
            0.5f, -0.5f, 0.5f,
            0.5f, -0.5f, 0.5f,
            -0.5f, -0.5f, 0.5f,
            -0.5f, -0.5f, -0.5f,

            -0.5f, 0.5f, -0.5f,
            0.5f, 0.5f, -0.5f,
            0.5f, 0.5f, 0.5f,
            0.5f, 0.5f, 0.5f,
            -0.5f, 0.5f, 0.5f,
            -0.5f, 0.5f, -0.5f,
    };
    // first, configure the cube's VAO (and VBO)
    glGenVertexArrays(1, &cubeVAO);
    glGenBuffers(1, &VBO);

    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

    glBindVertexArray(cubeVAO);

    // position attribute
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void *) 0);
    glEnableVertexAttribArray(0);

    // second, configure the light's VAO (VBO stays the same; the vertices are the same for the light object which is also a 3D cube)
    glGenVertexArrays(1, &lightVAO);
    glBindVertexArray(lightVAO);

    // we only need to bind to the VBO (to link it with glVertexAttribPointer), no need to fill it; the VBO's data already contains all we need (it's already bound, but we do it again for educational purposes)
    glBindBuffer(GL_ARRAY_BUFFER, VBO);

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void *) 0);
    glEnableVertexAttribArray(0);
    ALOGV("Using OpenGL ES 3.0 renderer");
    return true;
}

RendererES_light1::~RendererES_light1() {
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
    glDeleteVertexArrays(1, &cubeVAO);
    glDeleteBuffers(1, &VBO);
    glDeleteBuffers(1, &lightVAO);
}

float *RendererES_light1::mapOffsetBuf() {
    glBindBuffer(GL_ARRAY_BUFFER, mVB[VB_OFFSET]);
    return (float *) glMapBufferRange(GL_ARRAY_BUFFER,
                                      0, MAX_INSTANCES * 2 * sizeof(float),
                                      GL_MAP_WRITE_BIT | GL_MAP_INVALIDATE_BUFFER_BIT);
}

void RendererES_light1::unmapOffsetBuf() {
    glUnmapBuffer(GL_ARRAY_BUFFER);
}

float *RendererES_light1::mapTransformBuf() {
    glBindBuffer(GL_ARRAY_BUFFER, mVB[VB_SCALEROT]);
    return (float *) glMapBufferRange(GL_ARRAY_BUFFER,
                                      0, MAX_INSTANCES * 4 * sizeof(float),
                                      GL_MAP_WRITE_BIT | GL_MAP_INVALIDATE_BUFFER_BIT);
}

void RendererES_light1::unmapTransformBuf() {
    glUnmapBuffer(GL_ARRAY_BUFFER);
}

void RendererES_light1::draw(unsigned int numInstances) {
    // per-frame time logic
    // --------------------
    float currentFrame = getCurrentTime();
    float deltaTime = currentFrame - lastFrame;
    lastFrame = currentFrame;


    // render
    // ------
    glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    // be sure to activate shader when setting uniforms/drawing objects
    glUseProgram(lightingShader);
    setVec3(lightingShader, "objectColor", 1.0f, 0.5f, 0.31f);
    setVec3(lightingShader, "lightColor", 1.0f, 1.0f, 1.0f);

    // view/projection transformations
    glm::mat4 projection = glm::perspective(glm::radians(camera.Zoom),
                                            (float) SCR_WIDTH / (float) SCR_HEIGHT, 0.1f, 100.0f);
    glm::mat4 view = camera.GetViewMatrix();
    setMat4(lightingShader, "projection", projection);
    setMat4(lightingShader, "view", view);

    // world transformation
    glm::mat4 model;
    setMat4(lightingShader, "model", model);

    // render the cube
    glBindVertexArray(cubeVAO);
    glDrawArrays(GL_TRIANGLES, 0, 36);


    // also draw the lamp object
    glUseProgram(lampShader);
    setMat4(lampShader, "projection", projection);
    setMat4(lampShader, "view", view);
    model = glm::mat4();
    model = glm::translate(model, lightPos);

    model = glm::scale(model, glm::vec3(0.2f)); // a smaller cube
    setMat4(lampShader, "model", model);

    glBindVertexArray(lightVAO);
    glDrawArrays(GL_TRIANGLES, 0, 36);
}
