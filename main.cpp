//Autor: UROS POCEK
//Opis: Projekat1

#define _CRT_SECURE_NO_WARNINGS

#include <iostream>
#include <fstream>
#include <sstream>
#include <time.h>

#include <GL/glew.h>
#include <GLFW/glfw3.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include "model.hpp"
#include "shader.hpp"

#include <random>

#define STB_IMAGE_IMPLEMENTATION

enum FIRE_STATUS
{
    READY,
    TIMEOUT,
    OUT_OF_AMMO
};

enum HIDRAULIC_STATUS
{
    CHANGING,
    IDLE
};

unsigned int compileShader(GLenum type, const char* source);
unsigned int createShader(const char* vsSource, const char* fsSource);
static unsigned loadImageToTexture(const char* filePath);

// My functions
void drawLandscape(unsigned int moveShader, unsigned int VAO, unsigned landscapeTexture);
void drawSight(unsigned int basicShader, unsigned int VAO, unsigned sightTexture);
void drawFireStatusLight(unsigned int shader, unsigned int VAO, unsigned int wWidth, unsigned int wHight);
void drawAmmo(unsigned int shader, unsigned int VAO, unsigned int wWidth, unsigned int wHight, unsigned ammoTexture);
void drawVoltmeter(unsigned int shaderBG, unsigned int shaderFG, unsigned int VAO_BG, unsigned int VAO_FG, unsigned int wWidth, unsigned int wHight, unsigned voltmeterTexture, float voltmeterValue, unsigned uVoltmeterValueLoc);
void drawName(unsigned int shader, unsigned int VAO, unsigned int wWidth, unsigned int wHight, unsigned ammoTexture);
void setupTexture(unsigned texture);
void handleViewChange(GLFWwindow* window, bool* view_in_cockpit);
void handleHidraulic(GLFWwindow* window, float* voltmeter_value, float voltmeter_speed, float min_voltmeter_value, float max_voltmeter_value, bool* hidraulic_on, HIDRAULIC_STATUS* hidraulic_status, int* hidraulic_oscilation);
void drawTargets(unsigned int shader, unsigned int* VAOs, unsigned targetTexture, bool* tagets_hit);
bool is_target_hit(float tank_rotation, float target_rotation);

void start2D() {
    glDisable(GL_DEPTH_TEST);
    glDisable(GL_CULL_FACE);
}

void start3D() {
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_CULL_FACE);
}

float getRandomFloat(float min, float max) {
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_real_distribution<float> dis(min, max);
    return dis(gen);
}

int getRandomInt(int min, int max) {
    return min + (std::rand() % (max - min + 1));
}

int main(void)
{

    if (!glfwInit())
    {
        std::cout<<"GLFW Biblioteka se nije ucitala! :(\n";
        return 1;
    }

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    GLFWwindow* window;
    unsigned int wWidth = 1920;
    unsigned int wHeight = 1080;
    const char wTitle[] = "Uros Pocek SV57/2020";
    window = glfwCreateWindow(wWidth, wHeight, wTitle, NULL, NULL);
    
    if (window == NULL)
    {
        std::cout << "Prozor nije napravljen! :(\n";
        glfwTerminate();
        return 2;
    }

    glfwMakeContextCurrent(window);


    if (glewInit() != GLEW_OK)
    {
        std::cout << "GLEW nije mogao da se ucita! :'(\n";
        return 3;
    }

    unsigned int unifiedShader = createShader("basic.vert", "basic.frag");
    unsigned int lightShader = createShader("basic.vert", "light.frag");
    unsigned int arrowShader = createShader("arrow.vert", "arrow.frag");

    // LANDSCAPE - START

    float landscape[] =
    {   //X    Y      S    T 
        -1.0, -1.0,   0.0, 0.0,
         1.0, -1.0,   1.0, 0.0, 
         1.0, 1.0,    1.0, 1.0, 
        -1.0, 1.0,    0.0, 1.0
    };
    unsigned int landscape_indices[] = 
    {
        0, 1, 2,
        0, 2, 3,
    };
    unsigned int stride = (2 + 2) * sizeof(float);

    unsigned int VAO[6];
    glGenVertexArrays(6, VAO);
    glBindVertexArray(VAO[0]);
    unsigned int VBO;
    glGenBuffers(1, &VBO);
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(landscape), landscape, GL_STATIC_DRAW);
   
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, stride, (void*)0);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, stride, (void*)(2 * sizeof(float)));
    glEnableVertexAttribArray(1);

    unsigned int EBO;
    glGenBuffers(1, &EBO);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(landscape_indices), landscape_indices, GL_STATIC_DRAW);

    unsigned landscapeTexture = loadImageToTexture("res/landscape.jpg");
    setupTexture(landscapeTexture);

    // LANDSCAPE - END

    // COCKPIT - START

    unsigned cockpitTexture = loadImageToTexture("res/cockpit.jpg");
    setupTexture(cockpitTexture);

    // COCKPIT - END

    // SIGHT - START

    unsigned sightTexture = loadImageToTexture("res/sight.png");
    setupTexture(sightTexture);
    unsigned sightBlankTexture = loadImageToTexture("res/sight_blank.png");
    setupTexture(sightBlankTexture);

    // SIGHT - END

    // FIRE LIGHT - START

    float circle[361 * 2] = { 0.0 };
    circle[0] = 0.0;
    circle[1] = 0.0;


    float r = 0.5;
    for (int i = 0; i <= 360 * 2; i += 2) {
        circle[i] = sin((3.14 / 180.0) * (i / 2.0)) * r;
        circle[i + 1] = cos((3.14 / 180.0) * (i / 2.0)) * r;
    }

    glBindVertexArray(VAO[1]);

    unsigned int VBO2;
    glGenBuffers(1, &VBO2);
    glBindBuffer(GL_ARRAY_BUFFER, VBO2);
    glBufferData(GL_ARRAY_BUFFER, sizeof(circle), circle, GL_STATIC_DRAW); 
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);

    unsigned uLightColorLoc = glGetUniformLocation(lightShader, "uLightColor");

    // FIRE LIGHT - END

    // AMMO INDICATOR - START

    unsigned ammo6Texture = loadImageToTexture("res/ammo_6.png");
    setupTexture(ammo6Texture);
    unsigned ammo5Texture = loadImageToTexture("res/ammo_5.png");
    setupTexture(ammo5Texture);
    unsigned ammo4Texture = loadImageToTexture("res/ammo_4.png");
    setupTexture(ammo4Texture);
    unsigned ammo3Texture = loadImageToTexture("res/ammo_3.png");
    setupTexture(ammo3Texture);
    unsigned ammo2Texture = loadImageToTexture("res/ammo_2.png");
    setupTexture(ammo2Texture);
    unsigned ammo1Texture = loadImageToTexture("res/ammo_1.png");
    setupTexture(ammo1Texture);
    unsigned ammo0Texture = loadImageToTexture("res/ammo_0.png");
    setupTexture(ammo0Texture);

    // AMMO INTICATOR - END

    // NAME - START

    unsigned nameTexture = loadImageToTexture("res/name.png");
    setupTexture(nameTexture);

    // NAME - END

    // VOLTMETER - START

    unsigned voltmeterTexture = loadImageToTexture("res/voltmeter.png");
    setupTexture(voltmeterTexture);

    float arrow[] =
    {   //X    Y
         0.0, -0.26,
        -0.78,  0.4
    };

    glBindVertexArray(VAO[2]);
    unsigned int VBO3;
    glGenBuffers(1, &VBO3);
    glBindBuffer(GL_ARRAY_BUFFER, VBO3);
    glBufferData(GL_ARRAY_BUFFER, sizeof(arrow), arrow, GL_STATIC_DRAW);

    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);

    unsigned uVoltmeterValueLoc = glGetUniformLocation(arrowShader, "value");

    // VOLTMETER - END

    // TARGET - START

    srand((unsigned)time(0));
    // Get a random number between 0.0 and 0.25
    float random_t2 = (static_cast <float> (rand()) / static_cast <float> (RAND_MAX))/4.0;
    float random_t3 = (static_cast <float> (rand()) / static_cast <float> (RAND_MAX))/4.0;

    float target1[] =
    {   //X    Y      S    T 
        -0.1, -0.23,   0.0, 0.0,
         0.1, -0.23,   1.0, 0.0,
         0.1, 0.23,    1.0, 1.0,
        -0.1, 0.23,    0.0, 1.0
    };
    float target2[] =
    {   //X    Y      S    T 
        -0.29, -random_t2,   0.0, 0.0,
        -0.19, -random_t2,   1.0, 0.0,
        -0.19, (-random_t2 + 0.24),    1.0, 1.0,
        -0.29, (-random_t2 + 0.24),    0.0, 1.0
    };
    float target3[] =
    {   //X    Y      S    T 
         0.19, -random_t3,   0.0, 0.0,
         0.29, -random_t3,   1.0, 0.0,
         0.29, (-random_t3 + 0.24),    1.0, 1.0,
         0.19, (-random_t3 + 0.24),    0.0, 1.0
    };

    unsigned int target_indices[] =
    {
        0, 1, 2,
        0, 2, 3,
    };
    unsigned int stride_target = (2 + 2) * sizeof(float);

    glBindVertexArray(VAO[3]);
    unsigned int VBO4;
    glGenBuffers(1, &VBO4);
    glBindBuffer(GL_ARRAY_BUFFER, VBO4);
    glBufferData(GL_ARRAY_BUFFER, sizeof(target1), target1, GL_STATIC_DRAW);

    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, stride_target, (void*)0);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, stride_target, (void*)(2 * sizeof(float)));
    glEnableVertexAttribArray(1);

    unsigned int EBO2;
    glGenBuffers(1, &EBO2);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO2);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(target_indices), target_indices, GL_STATIC_DRAW);

    glBindVertexArray(VAO[4]);
    unsigned int VBO5;
    glGenBuffers(1, &VBO5);
    glBindBuffer(GL_ARRAY_BUFFER, VBO5);
    glBufferData(GL_ARRAY_BUFFER, sizeof(target2), target2, GL_STATIC_DRAW);

    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, stride_target, (void*)0);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, stride_target, (void*)(2 * sizeof(float)));
    glEnableVertexAttribArray(1);

    unsigned int EBO3;
    glGenBuffers(1, &EBO3);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO3);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(target_indices), target_indices, GL_STATIC_DRAW);

    glBindVertexArray(VAO[5]);
    unsigned int VBO6;
    glGenBuffers(1, &VBO6);
    glBindBuffer(GL_ARRAY_BUFFER, VBO6);
    glBufferData(GL_ARRAY_BUFFER, sizeof(target3), target3, GL_STATIC_DRAW);

    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, stride_target, (void*)0);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, stride_target, (void*)(2 * sizeof(float)));
    glEnableVertexAttribArray(1);

    unsigned int EBO4;
    glGenBuffers(1, &EBO4);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO4);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(target_indices), target_indices, GL_STATIC_DRAW);

    unsigned targetTexture = loadImageToTexture("res/target.png");
    setupTexture(targetTexture);

    // TARGET - END

    glBindTexture(GL_TEXTURE_2D, 0);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);
    unsigned uTexLoc = glGetUniformLocation(unifiedShader, "uTex");
    glUniform1i(uTexLoc, 0);

    glClearColor(0.5, 0.5, 0.5, 1.0);

    bool view_in_cockpit = false;
    bool hidraulic_on = true;
    FIRE_STATUS fire_status = FIRE_STATUS::READY;
    HIDRAULIC_STATUS hidraulic_status = HIDRAULIC_STATUS::IDLE;
    float offset_y = 0.0f;
    float offset_x = 0.0f;
    float max_offset_y = 100.0f;
    float default_movement_speed = 1.0f;
    float voltmeter_value = -0.78f;
    float max_voltmeter_value = 0.78f;
    float min_voltmeter_value = -0.78f;
    float voltmeter_speed = 0.002f;
    int number_of_ammunition = 6;
    int hidraulic_oscilation = 1;
    bool targets_hit[3] = { false, false, false };

    bool zoom_active = false;
    bool night_vision_active = false;
    bool reflector_active = true;
    float movement_speed;
    int target_move_direction[3] = {1,-1,1};
    float target_move_ammount[3] = { 0,0,0 };
    float target_check[3] = { 0,0,0 };
    float tank_rotation = 0.0f;

    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    // Light cube
    Shader fireCubeShader("cube.vert", "cube.frag");

    float cube_vertices[] = {
        -0.5f, -0.5f, -0.5f,
         0.5f, -0.5f, -0.5f,
         0.5f,  0.5f, -0.5f,
         0.5f,  0.5f, -0.5f,
        -0.5f,  0.5f, -0.5f,
        -0.5f, -0.5f, -0.5f,

        -0.5f, -0.5f,  0.5f,
         0.5f, -0.5f,  0.5f,
         0.5f,  0.5f,  0.5f,
         0.5f,  0.5f,  0.5f,
        -0.5f,  0.5f,  0.5f,
        -0.5f, -0.5f,  0.5f,

        -0.5f,  0.5f,  0.5f,
        -0.5f,  0.5f, -0.5f,
        -0.5f, -0.5f, -0.5f,
        -0.5f, -0.5f, -0.5f,
        -0.5f, -0.5f,  0.5f,
        -0.5f,  0.5f,  0.5f,

         0.5f,  0.5f,  0.5f,
         0.5f,  0.5f, -0.5f,
         0.5f, -0.5f, -0.5f,
         0.5f, -0.5f, -0.5f,
         0.5f, -0.5f,  0.5f,
         0.5f,  0.5f,  0.5f,

        -0.5f, -0.5f, -0.5f,
         0.5f, -0.5f, -0.5f,
         0.5f, -0.5f,  0.5f,
         0.5f, -0.5f,  0.5f,
        -0.5f, -0.5f,  0.5f,
        -0.5f, -0.5f, -0.5f,

        -0.5f,  0.5f, -0.5f,
         0.5f,  0.5f, -0.5f,
         0.5f,  0.5f,  0.5f,
         0.5f,  0.5f,  0.5f,
        -0.5f,  0.5f,  0.5f,
        -0.5f,  0.5f, -0.5f,
    };

    unsigned int fireCubeVAO, fireCubeVBO;
    glGenVertexArrays(1, &fireCubeVAO);
    glGenBuffers(1, &fireCubeVBO);
    glBindVertexArray(fireCubeVAO);

    glBindBuffer(GL_ARRAY_BUFFER, fireCubeVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(cube_vertices), cube_vertices, GL_STATIC_DRAW);

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);

    // 3D STARTS HERE

    Shader main3DShader("main_3d.vert", "main_3d.frag");

    glm::mat4 projection = glm::perspective(glm::radians(45.0f), (float)wWidth / (float)wHeight, 0.1f, 100.0f);
    glm::mat4 view = glm::lookAt(glm::vec3(0.01f, 3.0f, 2.01f), glm::vec3(0.0f, 3.0f, 0.0f), glm::vec3(0.0f, 1.0f, 0.0f));
    glm::vec3 firelightPos(-0.62, 3.15f, -2.7f);

    glm::mat4 model = glm::mat4(1.0f);
    glm::mat4 scene = glm::mat4(1.0f);
    glm::mat4 cannon = glm::mat4(1.0f);
    glm::mat4 targetMat = glm::mat4(1.0f);
    Model plane("res/plane.obj");
    Model tank("res/tank.obj");
    Model target("res/target.obj");

    cannon = glm::translate(cannon, glm::vec3(0.52f, 3.15f, -0.1f));
    
    main3DShader.use();
    main3DShader.setVec3("viewPos", 0.01f, 3.0f, 2.01f);

    float random_targets[] = { getRandomFloat(0.0, 0.33),  getRandomFloat(0.33, 0.66), getRandomFloat(0.66, 1.0) };

    // Lights
    // direction
    main3DShader.setVec3("dirLight.direction", 2.2f, -1.0f, 2.1f);
    main3DShader.setVec3("dirLight.ambient", 0.02f, 0.02f, 0.02f);
    main3DShader.setVec3("dirLight.diffuse", 0.06f, 0.06f, 0.07f);
    main3DShader.setVec3("dirLight.specular", 0.6f, 0.6f, 0.6f);
    //point
    main3DShader.setVec3("pointLights[0].position", firelightPos);
    main3DShader.setVec3("pointLights[0].ambient", 0.0f, 0.0f, 0.0f);
    main3DShader.setVec3("pointLights[0].diffuse", 0.0f, 0.0f, 0.0f);
    main3DShader.setVec3("pointLights[0].specular", 0.0f, 0.0f, 0.0f);
    main3DShader.setFloat("pointLights[0].constant", 0.01f);
    main3DShader.setFloat("pointLights[0].linear", 0.01f);
    main3DShader.setFloat("pointLights[0].quadratic", 0.01f);

    main3DShader.setVec3("pointLights[1].position", 0.0, 0.0, 0.0);
    main3DShader.setVec3("pointLights[1].ambient", 0.1f, 0.0f, 0.0f);
    main3DShader.setVec3("pointLights[1].diffuse", 0.6f, 0.0f, 0.0f);
    main3DShader.setVec3("pointLights[1].specular", 1.0f, 1.0f, 1.0f);
    main3DShader.setFloat("pointLights[1].constant", 0.01f);
    main3DShader.setFloat("pointLights[1].linear", 0.09f);
    main3DShader.setFloat("pointLights[1].quadratic", 0.032f);

    main3DShader.setVec3("pointLights[2].position", 0.0, 0.0, 0.0);
    main3DShader.setVec3("pointLights[2].ambient", 0.0f, 0.1f, 0.0f);
    main3DShader.setVec3("pointLights[2].diffuse", 0.0f, 0.6f, 0.0f);
    main3DShader.setVec3("pointLights[2].specular", 1.0f, 1.0f, 1.0f);
    main3DShader.setFloat("pointLights[2].constant", 0.01f);
    main3DShader.setFloat("pointLights[2].linear", 0.09f);
    main3DShader.setFloat("pointLights[2].quadratic", 0.032f);

    main3DShader.setVec3("pointLights[3].position", 0.0, 0.0, 0.0);
    main3DShader.setVec3("pointLights[3].ambient", 0.0f, 0.0f, 0.1f);
    main3DShader.setVec3("pointLights[3].diffuse", 0.0f, 0.0f, 0.6f);
    main3DShader.setVec3("pointLights[3].specular", 1.0f, 1.0f, 1.0f);
    main3DShader.setFloat("pointLights[3].constant", 0.01f);
    main3DShader.setFloat("pointLights[3].linear", 0.09f);
    main3DShader.setFloat("pointLights[3].quadratic", 0.032f);
    // spot
    main3DShader.setVec3("spotLight.position", 0.01f, 3.0f, 2.02f);
    main3DShader.setVec3("spotLight.direction", 0.0f, 0.0f, -1.0f);
    main3DShader.setFloat("spotLight.cutOff", 20.0f);
    main3DShader.setFloat("spotLight.outerCutOff", 30.0f);
    main3DShader.setVec3("spotLight.ambient", 0.1f, 0.1f, 0.1f);
    main3DShader.setVec3("spotLight.diffuse", 0.9f, 0.9f, 0.9f);
    main3DShader.setVec3("spotLight.specular", 1.0f, 1.0f, 1.0f);
    main3DShader.setFloat("spotLight.constant", 1.0f);
    main3DShader.setFloat("spotLight.linear", 0.014f);
    main3DShader.setFloat("spotLight.quadratic", 0.0007f);


    while (!glfwWindowShouldClose(window))
    {
        glClearColor(0.06, 0.06, 0.09, 1.0);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        // Drawing 3
        start3D();

        main3DShader.use();

        main3DShader.setMat4("uM", scene);
        main3DShader.setMat4("uV", view);
        main3DShader.setMat4("uP", projection);
        plane.Draw(main3DShader);

        main3DShader.setMat4("uM", cannon);
        tank.Draw(main3DShader);

        for (int i = 0; i < 3; i++) {
            if (targets_hit[i]) {
                continue;
            }
            targetMat = scene;

            float movement = getRandomInt(1, 200);
            float side = 0;
            if (movement == 100) {
                target_move_direction[i] *= -1;
            }
            if (i % 2 == 0) {
                side = 1;
            }
            else {
                side = -1;
            }
            target_move_ammount[i] += target_move_direction[i] * 0.005;

            targetMat = glm::rotate(targetMat, glm::radians(target_move_ammount[i]), glm::vec3(0.0f, 1.0f, 0.0f));
            targetMat = glm::translate(targetMat, glm::vec3(target_move_ammount[i], 0.0, 0.0));

            targetMat = glm::scale(targetMat, glm::vec3(0.75 + (0.25 * random_targets[i]), 0.75 + (0.25 * random_targets[i]), 0.75 + (0.25 * random_targets[i])));
            targetMat = glm::rotate(targetMat, glm::radians(180 * random_targets[i]), glm::vec3(0.0f, 1.0f, 0.0f));
            targetMat = glm::translate(targetMat, glm::vec3(side * 10.0 * random_targets[i], 0.0, -15.0 + (-10.0 * random_targets[i])));

            std::string pointLightIndex = "pointLights[" + std::to_string(i + 1) + "].position";

            glm::vec4 temp = targetMat * glm::vec4(0.0, 0.0, 0.0, 1.0);
            main3DShader.setVec3(pointLightIndex, temp.x, temp.y, temp.z);

            glm::vec3 tempVec(temp.x, temp.y, temp.z);
            glm::vec3 normalizedTempVec = glm::normalize(tempVec - glm::vec3(0.01, 3.0,2.01));

            float tempRotation = 360 - (180 * glm::acos(glm::dot(normalizedTempVec, glm::normalize(glm::vec3(0.0, 0.0, -1.0)))) / 3.14);
            if(target_check[i] == 0)
                target_check[i] = tempRotation;

            main3DShader.setMat4("uM", targetMat);
            target.Draw(main3DShader);
        }

        // Drawing 2D

        start2D();

        if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
        {
            glfwSetWindowShouldClose(window, GL_TRUE);
        }

        handleViewChange(window, &view_in_cockpit);

        handleHidraulic(window, &voltmeter_value, voltmeter_speed, min_voltmeter_value, max_voltmeter_value, &hidraulic_on, &hidraulic_status, &hidraulic_oscilation);

        if (!hidraulic_on) {
            movement_speed = default_movement_speed;
        }
        else {
            movement_speed = default_movement_speed + (abs(min_voltmeter_value) + voltmeter_value);
        }

        if (glfwGetKey(window, GLFW_KEY_LEFT) == GLFW_PRESS) {
            tank_rotation -= 0.1f * movement_speed;
            scene = glm::rotate(scene, glm::radians(-0.1f * movement_speed), glm::vec3(0.0f, 1.0f, 0.0f));
            if (tank_rotation < 0.0) {
                tank_rotation = 359.9;
            }
            if (tank_rotation > 360.0) {
                tank_rotation = 0.01;
            }
        }
        if (glfwGetKey(window, GLFW_KEY_RIGHT) == GLFW_PRESS) {
            tank_rotation += 0.1f * movement_speed;
            scene = glm::rotate(scene, glm::radians(0.1f * movement_speed), glm::vec3(0.0f, 1.0f, 0.0f));
            if (tank_rotation < 0) {
                tank_rotation = 359;
            }
            if (tank_rotation > 360) {
                tank_rotation = 0.01;
            }
        }

        if (glfwGetKey(window, GLFW_KEY_DOWN) == GLFW_PRESS) {
            if (!view_in_cockpit) {
                offset_y += movement_speed;
            }
            if (offset_y >= max_offset_y) {
                offset_y = max_offset_y;
            }
            else {
                cannon = glm::rotate(cannon, glm::radians(0.05f * -movement_speed), glm::vec3(1.0f, 0.0f, 0.0f));
                cannon = glm::translate(cannon, glm::vec3(0.0f, -movement_speed * 0.001, 0.0f));
            }
        }
        if (glfwGetKey(window, GLFW_KEY_UP) == GLFW_PRESS) {
            if (!view_in_cockpit) {
                offset_y -= movement_speed;
            }
            if (offset_y <= -max_offset_y) {
                offset_y = -max_offset_y;
            }
            else {
                cannon = glm::rotate(cannon, glm::radians(0.05f * movement_speed), glm::vec3(1.0f, 0.0f, 0.0f));
                cannon = glm::translate(cannon, glm::vec3(0.0f, movement_speed * 0.001, 0.0f));
            }
        }

        if (glfwGetKey(window, GLFW_KEY_Z) == GLFW_PRESS) {
            if (glfwGetTime() > 0.2) {
                if (zoom_active) {
                    projection = glm::perspective(glm::radians(45.0f), (float)wWidth / (float)wHeight, 0.1f, 100.0f);
                }
                else {
                    projection = glm::perspective(glm::radians(30.0f), (float)wWidth / (float)wHeight, 0.1f, 100.0f);
                }
                glfwSetTime(0);
                zoom_active = !zoom_active;
            }
        }

        if (glfwGetKey(window, GLFW_KEY_R) == GLFW_PRESS) {
            if (glfwGetTime() > 0.2) {
                if (reflector_active) {
                    main3DShader.setFloat("spotLight.cutOff", 0.0f);
                }
                else {
                    main3DShader.setFloat("spotLight.cutOff", 20.0f);
                }
                glfwSetTime(0);
                reflector_active = !reflector_active;
            }
        }

        if (glfwGetKey(window, GLFW_KEY_N) == GLFW_PRESS) {
            if (glfwGetTime() > 0.2) {
                main3DShader.use();
                if (night_vision_active) {
                    main3DShader.setVec3("dirLight.direction", 2.2f, -1.0f, 2.1f);
                    main3DShader.setVec3("dirLight.ambient", 0.02f, 0.02f, 0.02f);
                    main3DShader.setVec3("dirLight.diffuse", 0.06f, 0.06f, 0.07f);
                    main3DShader.setVec3("dirLight.specular", 0.8f, 0.8f, 0.8f);
                }
                else {
                    main3DShader.setVec3("dirLight.direction", 2.2f, -1.0f, 2.1f);
                    main3DShader.setVec3("dirLight.ambient", 0.0f, 0.5f, 0.0f);
                    main3DShader.setVec3("dirLight.diffuse", 0.0f, 0.0f, 0.0f);
                    main3DShader.setVec3("dirLight.specular", 1.0f, 1.0f, 1.0f);
                }
                glfwSetTime(0);
                night_vision_active = !night_vision_active;
            }
        }

        if (glfwGetKey(window, GLFW_KEY_SPACE) == GLFW_PRESS) {
            if (number_of_ammunition > 0 && fire_status == FIRE_STATUS::READY && !view_in_cockpit) {
                fire_status = FIRE_STATUS::TIMEOUT;

                // Fire light START
                fireCubeShader.use();
                fireCubeShader.setVec3("color", 1.0f, 1.0f, 0.6f);
                fireCubeShader.setFloat("oppacity", 0.5);
                fireCubeShader.setMat4("projection", projection);
                fireCubeShader.setMat4("view", view);

                glBindVertexArray(fireCubeVAO);
                model = glm::mat4(1.0f);
                model = glm::translate(model, firelightPos);
                model = glm::scale(model, glm::vec3(0.2f));
                fireCubeShader.setMat4("model", model);

                glDrawArrays(GL_TRIANGLES, 0, 36);

                //smoke
                fireCubeShader.setVec3("color", 1.0f, 1.0f, 1.0f);
                fireCubeShader.setFloat("oppacity", 1.0);
                model = glm::scale(model, glm::vec3(2.0f));
                fireCubeShader.setMat4("model", model);
                glDrawArrays(GL_TRIANGLES, 0, 36);

                main3DShader.use();
                main3DShader.setVec3("pointLights[0].ambient", 0.99f, 0.99f, 0.99f);
                main3DShader.setVec3("pointLights[0].diffuse", 0.99f, 0.99f, 0.99f);
                main3DShader.setVec3("pointLights[0].specular", 1.0f, 1.0f, 1.0f);
                // Fire light END

                glfwSetTime(0);
                number_of_ammunition -= 1;

                if (targets_hit[0]==false && is_target_hit(tank_rotation, target_check[0])) {
                    targets_hit[0] = true;
                    main3DShader.use();
                    main3DShader.setVec3("pointLights[1].ambient", 0.0f, 0.0f, 0.0f);
                    main3DShader.setVec3("pointLights[1].diffuse", 0.0f, 0.0f, 0.0f);
                    main3DShader.setVec3("pointLights[1].specular", 0.0f, 0.0f, 0.0f);
                }
                if (targets_hit[1] == false && is_target_hit(tank_rotation, target_check[1])) {
                    targets_hit[1] = true;
                    main3DShader.use();
                    main3DShader.setVec3("pointLights[2].ambient", 0.0f, 0.0f, 0.0f);
                    main3DShader.setVec3("pointLights[2].diffuse", 0.0f, 0.0f, 0.0f);
                    main3DShader.setVec3("pointLights[2].specular", 0.0f, 0.0f, 0.0f);
                }
                if (targets_hit[2] == false && is_target_hit(tank_rotation, target_check[2])) {
                    targets_hit[2] = true;
                    main3DShader.use();
                    main3DShader.setVec3("pointLights[3].ambient", 0.0f, 0.0f, 0.0f);
                    main3DShader.setVec3("pointLights[3].diffuse", 0.0f, 0.0f, 0.0f);
                    main3DShader.setVec3("pointLights[3].specular", 0.0f, 0.0f, 0.0f);
                }
            }
            if (number_of_ammunition <= 0) {
                fire_status = FIRE_STATUS::OUT_OF_AMMO;
            }
        }
        if (fire_status == FIRE_STATUS::TIMEOUT) {
            //smoke
            fireCubeShader.use();
            fireCubeShader.setVec3("color", 1.0f, 1.0f, 1.0f);
            fireCubeShader.setFloat("oppacity", 0.6f - glfwGetTime() / 2.0f);
            model = glm::mat4(1.0f);
            model = glm::translate(model, firelightPos);
            model = glm::scale(model, glm::vec3(glfwGetTime()));
            fireCubeShader.setMat4("model", model);
            glBindVertexArray(fireCubeVAO);
            glDrawArrays(GL_TRIANGLES, 0, 36);

            if (glfwGetTime() > 0.1) {
                main3DShader.use();
                main3DShader.setVec3("pointLights[0].ambient", 0.0f, 0.0f, 0.0f);
                main3DShader.setVec3("pointLights[0].diffuse", 0.0f, 0.0f, 0.0f);
                main3DShader.setVec3("pointLights[0].specular", 0.0f, 0.0f, 0.0f);
            }
            else {
                fireCubeShader.use();
                fireCubeShader.setVec3("color", 1.0f, 1.0f, 0.6f);
                fireCubeShader.setFloat("oppacity", 0.5);
                fireCubeShader.setMat4("projection", projection);
                fireCubeShader.setMat4("view", view);

                glBindVertexArray(fireCubeVAO);
                model = glm::mat4(1.0f);
                model = glm::translate(model, firelightPos);
                model = glm::scale(model, glm::vec3(0.2f));
                fireCubeShader.setMat4("model", model);

                glDrawArrays(GL_TRIANGLES, 0, 36);
            }
        }
        if (glfwGetTime() > 7.5) {
            if (number_of_ammunition > 0) {
                fire_status = FIRE_STATUS::READY;
            }
            else {
                fire_status = FIRE_STATUS::OUT_OF_AMMO;
            }
            hidraulic_status = HIDRAULIC_STATUS::IDLE;
        }

        glUseProgram(lightShader);
        if (fire_status == FIRE_STATUS::READY) {
            glUniform3f(uLightColorLoc, 1.0, 1.0, 0.0);
        }
        else if (fire_status == FIRE_STATUS::OUT_OF_AMMO) {
            glUniform3f(uLightColorLoc, 255.0 / 255.0, 51.0 / 255.0, 51.0 / 255.0);
        }
        else {
            glUniform3f(uLightColorLoc, 0.2, 0.2, 0.2);
        }

        if (!view_in_cockpit) {
            drawSight(unifiedShader, VAO[0], sightTexture);
        }
        else {
            drawLandscape(unifiedShader, VAO[0], cockpitTexture);
            drawSight(unifiedShader, VAO[0], sightBlankTexture);
        }

        drawFireStatusLight(lightShader, VAO[1], wWidth, wHeight);
        drawVoltmeter(unifiedShader, arrowShader, VAO[0], VAO[2], wWidth, wHeight, voltmeterTexture, voltmeter_value, uVoltmeterValueLoc);

        unsigned ammoTexture;

        switch (number_of_ammunition)
        {
        case 6: ammoTexture = ammo6Texture; break;
        case 5: ammoTexture = ammo5Texture; break;
        case 4: ammoTexture = ammo4Texture; break;
        case 3: ammoTexture = ammo3Texture; break;
        case 2: ammoTexture = ammo2Texture; break;
        case 1: ammoTexture = ammo1Texture; break;
        case 0: ammoTexture = ammo0Texture; break;
        default:
            break;
        }
        drawAmmo(unifiedShader, VAO[0], wWidth, wHeight, ammoTexture);
        drawName(unifiedShader, VAO[0], wWidth, wHeight, nameTexture);

        glfwSwapBuffers(window);
        glfwPollEvents();
    }


    glDeleteTextures(1, &landscapeTexture);
    glDeleteTextures(1, &cockpitTexture);
    glDeleteTextures(1, &sightTexture);
    glDeleteTextures(1, &sightBlankTexture);
    glDeleteTextures(1, &ammo0Texture);
    glDeleteTextures(1, &ammo1Texture);
    glDeleteTextures(1, &ammo2Texture);
    glDeleteTextures(1, &ammo3Texture);
    glDeleteTextures(1, &ammo4Texture);
    glDeleteTextures(1, &ammo5Texture);
    glDeleteTextures(1, &ammo6Texture);
    glDeleteTextures(1, &nameTexture);
    glDeleteTextures(1, &voltmeterTexture);
    glDeleteTextures(1, &targetTexture);
    glDeleteBuffers(1, &VBO);
    glDeleteBuffers(1, &VBO2);
    glDeleteBuffers(1, &VBO3);
    glDeleteBuffers(1, &VBO4);
    glDeleteBuffers(1, &VBO5);
    glDeleteBuffers(1, &VBO6);
    glDeleteVertexArrays(6, VAO);
    glDeleteVertexArrays(1, &fireCubeVAO);
    glDeleteProgram(unifiedShader);
    glDeleteProgram(lightShader);
    glDeleteProgram(arrowShader);

    glfwTerminate();
    return 0;
}

bool is_target_hit(float tank_rotation, float target_rotation) {
    if (tank_rotation > 340 && target_rotation < 20) {
        return true;
    }
    if (tank_rotation < 20 && target_rotation > 340) {
        return true;
    }
    return abs(tank_rotation - target_rotation) < 20;
}

void handleViewChange(GLFWwindow* window, bool *view_in_cockpit) {
    if (glfwGetKey(window, GLFW_KEY_C) == GLFW_PRESS) {
        *view_in_cockpit = true;
    }
    if (glfwGetKey(window, GLFW_KEY_V) == GLFW_PRESS) {
        *view_in_cockpit = false;
    }
}

void handleHidraulic(GLFWwindow* window, float *voltmeter_value, float voltmeter_speed, float min_voltmeter_value, float max_voltmeter_value, bool *hidraulic_on, HIDRAULIC_STATUS *hidraulic_status, int *hidraulic_oscilation) {
    if (*hidraulic_on) {
        *voltmeter_value += 0.04 * (*hidraulic_oscilation);
        if (*voltmeter_value >= max_voltmeter_value) {
            *voltmeter_value = max_voltmeter_value;
        }
        else if (*voltmeter_value <= min_voltmeter_value) {
            *voltmeter_value = min_voltmeter_value;
        }
        *hidraulic_oscilation *= -1;
    }
    
    if (glfwGetKey(window, GLFW_KEY_O) == GLFW_PRESS) {
        if (*hidraulic_on) {
            *voltmeter_value -= voltmeter_speed;
            if (*voltmeter_value <= min_voltmeter_value) {
                *voltmeter_value = min_voltmeter_value;
            }
        }
        
    }
    if (glfwGetKey(window, GLFW_KEY_P) == GLFW_PRESS) {
        if (*hidraulic_on) {
            *voltmeter_value += voltmeter_speed;
            if (*voltmeter_value >= max_voltmeter_value) {
                *voltmeter_value = max_voltmeter_value;
            }
        }
    }
    if (glfwGetKey(window, GLFW_KEY_H) == GLFW_PRESS) {
        if (*hidraulic_status != HIDRAULIC_STATUS::CHANGING) {
            *hidraulic_status = HIDRAULIC_STATUS::CHANGING;
            *hidraulic_on = !*hidraulic_on;
            if (!*hidraulic_on) {
                *voltmeter_value = min_voltmeter_value;
            }
            glfwSetTime(0);
        }
        if (glfwGetTime() > 1.5) {
            *hidraulic_status = HIDRAULIC_STATUS::IDLE;
        }
    }
}

void drawVoltmeter(unsigned int shaderBG, unsigned int shaderFG, unsigned int VAO_BG, unsigned int VAO_FG, unsigned int wWidth, unsigned int wHight, unsigned voltmeterTexture, float voltmeterValue, unsigned uVoltmeterValueLoc) {
    glViewport(wWidth - 380, 60, 250, 250);
    glUseProgram(shaderBG);
    glBindVertexArray(VAO_BG);
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, voltmeterTexture);
    glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, (void*)(0 * sizeof(unsigned int)));
    glBindTexture(GL_TEXTURE_2D, 0);

    glUseProgram(shaderFG);
    glUniform2f(uVoltmeterValueLoc, voltmeterValue, 0.4);
    glBindVertexArray(VAO_FG);
    glEnable(GL_PROGRAM_POINT_SIZE);
    glLineWidth(5.0);
    glDrawArrays(GL_LINES, 0, 2);

    glViewport(0, 0, wWidth, wHight);
}

void drawTargets(unsigned int shader, unsigned int *VAOs, unsigned targetTexture, bool *tagets_hit) {
    glUseProgram(shader);
    for (int i = 0; i < 3; i++) {
        if (!tagets_hit[i]) {
            glBindVertexArray(VAOs[i]);
            glActiveTexture(GL_TEXTURE0);
            glBindTexture(GL_TEXTURE_2D, targetTexture);
            glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, (void*)(0 * sizeof(unsigned int)));
        }
    }
    glBindTexture(GL_TEXTURE_2D, 0);
    glBindVertexArray(0);
}

void drawAmmo(unsigned int shader, unsigned int VAO, unsigned int wWidth, unsigned int wHight, unsigned ammoTexture) {
    glViewport(wWidth - 248, wHight - 730, 216, 160);
    glUseProgram(shader);
    glBindVertexArray(VAO);
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, ammoTexture);
    glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, (void*)(0 * sizeof(unsigned int)));
    glBindTexture(GL_TEXTURE_2D, 0);
    glBindVertexArray(0);
    glViewport(0, 0, wWidth, wHight);
}

void drawFireStatusLight(unsigned int shader, unsigned int VAO, unsigned int wWidth, unsigned int wHight) {
    glViewport(30, wHight - 150, 120, 120);
    glUseProgram(shader);
    glBindVertexArray(VAO);
    glDrawArrays(GL_TRIANGLE_FAN, 0, 361);
    glBindVertexArray(0);
    glViewport(0, 0, wWidth, wHight);
}

void drawSight(unsigned int shader, unsigned int VAO, unsigned sightTexture) {
    glUseProgram(shader);
    glBindVertexArray(VAO);
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, sightTexture);
    glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, (void*)(0 * sizeof(unsigned int)));
    glBindTexture(GL_TEXTURE_2D, 0);
    glBindVertexArray(0);
}

void drawLandscape(unsigned int shader, unsigned int VAO, unsigned landscapeTexture) {
    glUseProgram(shader);
    glBindVertexArray(VAO);
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, landscapeTexture);
    glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, (void*)(0 * sizeof(unsigned int)));
    glBindTexture(GL_TEXTURE_2D, 0);
    glBindVertexArray(0);
}

void drawName(unsigned int shader, unsigned int VAO, unsigned int wWidth, unsigned int wHight, unsigned ammoTexture) {
    glViewport(50, 90, 220, 28);
    glUseProgram(shader);
    glBindVertexArray(VAO);
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, ammoTexture);
    glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, (void*)(0 * sizeof(unsigned int)));
    glBindTexture(GL_TEXTURE_2D, 0);
    glBindVertexArray(0);
    glViewport(0, 0, wWidth, wHight);
}

void setupTexture(unsigned texture) {
    glBindTexture(GL_TEXTURE_2D, texture);
    glGenerateMipmap(GL_TEXTURE_2D);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
}

unsigned int compileShader(GLenum type, const char* source)
{
    std::string content = "";
    std::ifstream file(source);
    std::stringstream ss;
    if (file.is_open())
    {
        ss << file.rdbuf();
        file.close();
        std::cout << "Uspjesno procitao fajl sa putanje \"" << source << "\"!" << std::endl;
    }
    else {
        ss << "";
        std::cout << "Greska pri citanju fajla sa putanje \"" << source << "\"!" << std::endl;
    }
     std::string temp = ss.str();
     const char* sourceCode = temp.c_str();

    int shader = glCreateShader(type);
    
    int success;
    char infoLog[512];
    glShaderSource(shader, 1, &sourceCode, NULL);
    glCompileShader(shader);

    glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
    if (success == GL_FALSE)
    {
        glGetShaderInfoLog(shader, 512, NULL, infoLog);
        if (type == GL_VERTEX_SHADER)
            printf("VERTEX");
        else if (type == GL_FRAGMENT_SHADER)
            printf("FRAGMENT");
        printf(" sejder ima gresku! Greska: \n");
        printf(infoLog);
    }
    return shader;
}
unsigned int createShader(const char* vsSource, const char* fsSource)
{
    
    unsigned int program;
    unsigned int vertexShader;
    unsigned int fragmentShader;

    program = glCreateProgram();

    vertexShader = compileShader(GL_VERTEX_SHADER, vsSource);
    fragmentShader = compileShader(GL_FRAGMENT_SHADER, fsSource);

    
    glAttachShader(program, vertexShader);
    glAttachShader(program, fragmentShader);

    glLinkProgram(program);
    glValidateProgram(program);

    int success;
    char infoLog[512];
    glGetProgramiv(program, GL_VALIDATE_STATUS, &success);
    if (success == GL_FALSE)
    {
        glGetShaderInfoLog(program, 512, NULL, infoLog);
        std::cout << "Objedinjeni sejder ima gresku! Greska: \n";
        std::cout << infoLog << std::endl;
    }

    glDetachShader(program, vertexShader);
    glDeleteShader(vertexShader);
    glDetachShader(program, fragmentShader);
    glDeleteShader(fragmentShader);

    return program;
}
static unsigned loadImageToTexture(const char* filePath) {
    int TextureWidth;
    int TextureHeight;
    int TextureChannels;
    unsigned char* ImageData = stbi_load(filePath, &TextureWidth, &TextureHeight, &TextureChannels, 0);
    if (ImageData != NULL)
    {
        //Slike se osnovno ucitavaju naopako pa se moraju ispraviti da budu uspravne
        stbi__vertical_flip(ImageData, TextureWidth, TextureHeight, TextureChannels);

        // Provjerava koji je format boja ucitane slike
        GLint InternalFormat = -1;
        switch (TextureChannels) {
        case 1: InternalFormat = GL_RED; break;
        case 3: InternalFormat = GL_RGB; break;
        case 4: InternalFormat = GL_RGBA; break;
        default: InternalFormat = GL_RGB; break;
        }

        unsigned int Texture;
        glGenTextures(1, &Texture);
        glBindTexture(GL_TEXTURE_2D, Texture);
        glTexImage2D(GL_TEXTURE_2D, 0, InternalFormat, TextureWidth, TextureHeight, 0, InternalFormat, GL_UNSIGNED_BYTE, ImageData);
        glBindTexture(GL_TEXTURE_2D, 0);
        // oslobadjanje memorije zauzete sa stbi_load posto vise nije potrebna
        stbi_image_free(ImageData);
        return Texture;
    }
    else
    {
        std::cout << "Textura nije ucitana! Putanja texture: " << filePath << std::endl;
        stbi_image_free(ImageData);
        return 0;
    }
}