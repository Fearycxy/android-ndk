#include "gles3jni.h"
#include <EGL/egl.h>
#include "utils.h"
#include "include/camera.h"

#define STB_IMAGE_IMPLEMENTATION

#include "stb_image.h"

#define STR(s) #s
#define STRV(s) STR(s)

#define POS_ATTRIB 0
#define COLOR_ATTRIB 1
#define SCALEROT_ATTRIB 2
#define OFFSET_ATTRIB 3

static const char LIGHT_VERTEX_SHADER[] =
        "#version 300 es\n"
                "layout (location = 0) in vec3 aPos;\n"
                "layout (location = 1) in vec3 aNormal;\n"
                "\n"
                "out vec3 FragPos;\n"
                "out vec3 Normal;\n"
                "\n"
                "uniform mat4 model;\n"
                "uniform mat4 view;\n"
                "uniform mat4 projection;\n"
                "\n"
                "void main()\n"
                "{\n"
                "    FragPos = vec3(model * vec4(aPos, 1.0));\n"
                "    Normal = aNormal;  \n"
                "    \n"
                "    gl_Position = projection * view * vec4(FragPos, 1.0);\n"
                "}";

static const char LIGHT_FRAGMENT_SHADER[] =
        "#version 300 es\n"
                "precision mediump float;\n"
                "out vec4 FragColor;\n"
                "\n"
                "in vec3 Normal;  \n"
                "in vec3 FragPos;  \n"
                "  \n"
                "uniform vec3 lightPos; \n"
                "uniform vec3 lightColor;\n"
                "uniform vec3 objectColor;\n"
                "\n"
                "void main()\n"
                "{\n"
                "    // ambient\n"
                "    float ambientStrength = 0.1;\n"
                "    vec3 ambient = ambientStrength * lightColor;\n"
                "  \t\n"
                "    // diffuse \n"
                "    vec3 norm = normalize(Normal);\n"
                "    vec3 lightDir = normalize(lightPos - FragPos);\n"
                "    float diff = max(dot(norm, lightDir), 0.0);\n"
                "    vec3 diffuse = diff * lightColor;\n"
                "            \n"
                "    vec3 result = (ambient + diffuse) * objectColor;\n"
                "    FragColor = vec4(result, 1.0);\n"
                "}";

static const char VERTEX_SHADER[] =
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

static const char FRAGMENT_SHADER[] =
        "#version 300 es\n"
                "precision mediump float;\n"
                "out vec4 FragColor;\n"
                "\n"
                "void main()\n"
                "{\n"
                "    FragColor = vec4(1.0); // set alle 4 vector values to 1.0\n"
                "}";

class Render_3D : public Renderer {
public:
    Render_3D();

    virtual ~Render_3D();

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
    GLuint lightingShader;
    GLuint lampShader;
    GLuint mVB[VB_COUNT];
    GLuint mVBState;
};

Renderer *createES4Renderer() {
    Render_3D *renderer = new Render_3D;
    if (!renderer->init()) {
        delete renderer;
        return NULL;
    }
    return renderer;
}

Render_3D::Render_3D()
        : mEglContext(eglGetCurrentContext()),
          mProgram(0),
          mVBState(0) {
    for (int i = 0; i < VB_COUNT; i++)
        mVB[i] = 0;
}

unsigned int VBO, VAO, EBO, texture, cubeVAO, lightVAO;
unsigned int texture1, texture2;
// timing
float deltaTime = 0.0f;
float lastFrame = 0.0f;

// lighting
glm::vec3 lightPos(1.2f, 1.0f, 2.0f);

bool Render_3D::init() {
    lightingShader = createProgram(LIGHT_VERTEX_SHADER, LIGHT_FRAGMENT_SHADER);
    lampShader = createProgram(VERTEX_SHADER, FRAGMENT_SHADER);
    if (!lightingShader || !lampShader)
        return false;
    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LESS);
    // set up vertex data (and buffer(s)) and configure vertex attributes
    // ------------------------------------------------------------------
    // ------------------------------------------------------------------
    float vertices[] = {
            -0.5f, -0.5f, -0.5f, 0.0f, 0.0f, -1.0f,
            0.5f, -0.5f, -0.5f, 0.0f, 0.0f, -1.0f,
            0.5f, 0.5f, -0.5f, 0.0f, 0.0f, -1.0f,
            0.5f, 0.5f, -0.5f, 0.0f, 0.0f, -1.0f,
            -0.5f, 0.5f, -0.5f, 0.0f, 0.0f, -1.0f,
            -0.5f, -0.5f, -0.5f, 0.0f, 0.0f, -1.0f,

            -0.5f, -0.5f, 0.5f, 0.0f, 0.0f, 1.0f,
            0.5f, -0.5f, 0.5f, 0.0f, 0.0f, 1.0f,
            0.5f, 0.5f, 0.5f, 0.0f, 0.0f, 1.0f,
            0.5f, 0.5f, 0.5f, 0.0f, 0.0f, 1.0f,
            -0.5f, 0.5f, 0.5f, 0.0f, 0.0f, 1.0f,
            -0.5f, -0.5f, 0.5f, 0.0f, 0.0f, 1.0f,

            -0.5f, 0.5f, 0.5f, -1.0f, 0.0f, 0.0f,
            -0.5f, 0.5f, -0.5f, -1.0f, 0.0f, 0.0f,
            -0.5f, -0.5f, -0.5f, -1.0f, 0.0f, 0.0f,
            -0.5f, -0.5f, -0.5f, -1.0f, 0.0f, 0.0f,
            -0.5f, -0.5f, 0.5f, -1.0f, 0.0f, 0.0f,
            -0.5f, 0.5f, 0.5f, -1.0f, 0.0f, 0.0f,

            0.5f, 0.5f, 0.5f, 1.0f, 0.0f, 0.0f,
            0.5f, 0.5f, -0.5f, 1.0f, 0.0f, 0.0f,
            0.5f, -0.5f, -0.5f, 1.0f, 0.0f, 0.0f,
            0.5f, -0.5f, -0.5f, 1.0f, 0.0f, 0.0f,
            0.5f, -0.5f, 0.5f, 1.0f, 0.0f, 0.0f,
            0.5f, 0.5f, 0.5f, 1.0f, 0.0f, 0.0f,

            -0.5f, -0.5f, -0.5f, 0.0f, -1.0f, 0.0f,
            0.5f, -0.5f, -0.5f, 0.0f, -1.0f, 0.0f,
            0.5f, -0.5f, 0.5f, 0.0f, -1.0f, 0.0f,
            0.5f, -0.5f, 0.5f, 0.0f, -1.0f, 0.0f,
            -0.5f, -0.5f, 0.5f, 0.0f, -1.0f, 0.0f,
            -0.5f, -0.5f, -0.5f, 0.0f, -1.0f, 0.0f,

            -0.5f, 0.5f, -0.5f, 0.0f, 1.0f, 0.0f,
            0.5f, 0.5f, -0.5f, 0.0f, 1.0f, 0.0f,
            0.5f, 0.5f, 0.5f, 0.0f, 1.0f, 0.0f,
            0.5f, 0.5f, 0.5f, 0.0f, 1.0f, 0.0f,
            -0.5f, 0.5f, 0.5f, 0.0f, 1.0f, 0.0f,
            -0.5f, 0.5f, -0.5f, 0.0f, 1.0f, 0.0f
    };
    // first, configure the cube's VAO (and VBO)
    glGenVertexArrays(1, &cubeVAO);
    glGenBuffers(1, &VBO);

    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

    glBindVertexArray(cubeVAO);

    // position attribute
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void *) 0);
    glEnableVertexAttribArray(0);
    // normal attribute
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float),
                          (void *) (3 * sizeof(float)));
    glEnableVertexAttribArray(1);


    // second, configure the light's VAO (VBO stays the same; the vertices are the same for the light object which is also a 3D cube)
    glGenVertexArrays(1, &lightVAO);
    glBindVertexArray(lightVAO);

    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    // note that we update the lamp's position attribute's stride to reflect the updated buffer data
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void *) 0);
    glEnableVertexAttribArray(0);


    ALOGV("Using OpenGL ES 3.0 renderer");
    return true;
}

Render_3D::~Render_3D() {
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
    glDeleteVertexArrays(1, &lightVAO);
    glDeleteBuffers(1, &VBO);
}

float *Render_3D::mapOffsetBuf() {
    glBindBuffer(GL_ARRAY_BUFFER, mVB[VB_OFFSET]);
    return (float *) glMapBufferRange(GL_ARRAY_BUFFER,
                                      0, MAX_INSTANCES * 2 * sizeof(float),
                                      GL_MAP_WRITE_BIT | GL_MAP_INVALIDATE_BUFFER_BIT);
}

void Render_3D::unmapOffsetBuf() {
    glUnmapBuffer(GL_ARRAY_BUFFER);
}

float *Render_3D::mapTransformBuf() {
    glBindBuffer(GL_ARRAY_BUFFER, mVB[VB_SCALEROT]);
    return (float *) glMapBufferRange(GL_ARRAY_BUFFER,
                                      0, MAX_INSTANCES * 4 * sizeof(float),
                                      GL_MAP_WRITE_BIT | GL_MAP_INVALIDATE_BUFFER_BIT);
}

void Render_3D::unmapTransformBuf() {
    glUnmapBuffer(GL_ARRAY_BUFFER);
}

const unsigned int SCR_WIDTH = 800;
const unsigned int SCR_HEIGHT = 600;
Camera camera(glm::vec3(0.0f, 0.0f, 3.0f));
long gDegreesRotated = 0;


void Render_3D::draw(unsigned int numInstances) {
    static float currentFrame = 0;
    currentFrame++;
    ALOGV("currentFrame: %f", currentFrame);
    deltaTime = currentFrame - lastFrame;
    lastFrame = currentFrame;

    // render
    // ------
    glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    // be sure to activate shader when setting uniforms/drawing objects
    glUseProgram(lightingShader);
    setVec3(lightingShader, "objectColor", 1.0f, 0.5f, 0.31f);
    setVec3(lightingShader, "lightColor", 1.0f, 1.0f, 1.0f);
    setVec3(lightingShader, "lightPos", lightPos);

    // view/projection transformations
    glm::mat4 projection = glm::perspective(glm::radians(camera.Zoom),
                                            (float) SCR_WIDTH / (float) SCR_HEIGHT, 0.1f, 100.0f);
    glm::mat4 view = camera.GetViewMatrix();
    setMat4(lightingShader, "projection", projection);
    setMat4(lightingShader, "view", view);

    // world transformation
    glm::mat4 model(1.0f);
    model = rotate(model, (float) glm::radians(currentFrame), glm::vec3(1.0f, 1.0f, 1.0f));
    setMat4(lightingShader, "model", model);

    // render the cube
    glBindVertexArray(cubeVAO);
    glDrawArrays(GL_TRIANGLES, 0, 36);


    // also draw the lamp object
    glUseProgram(lampShader);
    setMat4(lampShader, "projection", projection);
    setMat4(lampShader, "view", view);
    model = glm::mat4(1.0f);
    model = glm::translate(model, lightPos);
    model = glm::scale(model, glm::vec3(0.2f)); // a smaller cube
    setMat4(lampShader, "model", model);

    glBindVertexArray(lightVAO);
    glDrawArrays(GL_TRIANGLES, 0, 36);


}
