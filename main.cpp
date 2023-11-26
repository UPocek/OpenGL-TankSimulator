//Autor: UROS POCEK
//Opis: Projekat1

#define _CRT_SECURE_NO_WARNINGS

#include <iostream>
#include <fstream>
#include <sstream>
#include <time.h>

#include <GL/glew.h>
#include <GLFW/glfw3.h>

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

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
void handleMove(GLFWwindow* window, bool hidraulic_on, bool view_in_cockpit, float default_movement_speed, float min_voltmeter_value, float voltmeter_value, float* offset_x, float max_offset_x, float* offset_y, float max_offset_y);
void handleViewChange(GLFWwindow* window, bool* view_in_cockpit);
void handleHidraulic(GLFWwindow* window, float* voltmeter_value, float voltmeter_speed, float min_voltmeter_value, float max_voltmeter_value, bool* hidraulic_on, HIDRAULIC_STATUS* hidraulic_status, int* hidraulic_oscilation);
void drawTargets(unsigned int shader, unsigned int* VAOs, unsigned targetTexture, bool* tagets_hit);
bool is_target_hit(float offset_x, float offset_y, float* target);

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
    unsigned int moveShader = createShader("move.vert", "basic.frag");
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

    unsigned uOffsetLoc = glGetUniformLocation(moveShader, "uOffset");

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
    float offset_x = 0;
    float offset_y = 0;
    float max_offset_x = 0.26;
    float max_offset_y = 0.1;
    float default_movement_speed = 0.001;
    float voltmeter_value = -0.78;
    float max_voltmeter_value = 0.78;
    float min_voltmeter_value = -0.78;
    float voltmeter_speed = 0.002;
    int number_of_ammunition = 6;
    int hidraulic_oscilation = 1;
    bool targets_hit[3] = { false, false, false };

    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    while (!glfwWindowShouldClose(window))
    {
        glClearColor(0.0, 0.0, 0.0, 1.0);
        glClear(GL_COLOR_BUFFER_BIT);
        
        if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
        {
            glfwSetWindowShouldClose(window, GL_TRUE);
        }

        handleViewChange(window, &view_in_cockpit);

        handleHidraulic(window, &voltmeter_value, voltmeter_speed, min_voltmeter_value, max_voltmeter_value, &hidraulic_on, &hidraulic_status, &hidraulic_oscilation);

        handleMove(window, hidraulic_on, view_in_cockpit, default_movement_speed, min_voltmeter_value, voltmeter_value, &offset_x, max_offset_x, &offset_y, max_offset_y);

        if (glfwGetKey(window, GLFW_KEY_SPACE) == GLFW_PRESS) {
            if (number_of_ammunition > 0 && fire_status == FIRE_STATUS::READY && !view_in_cockpit) {
                fire_status = FIRE_STATUS::TIMEOUT;
                glfwSetTime(0);
                number_of_ammunition -= 1;
                if (is_target_hit(offset_x, offset_y, target1)) {
                    targets_hit[0] = true;
                }
                if (is_target_hit(offset_x, offset_y, target2)) {
                    targets_hit[1] = true;
                }
                if (is_target_hit(offset_x, offset_y, target3)) {
                    targets_hit[2] = true;
                }
            }
            if(number_of_ammunition <= 0) {
                fire_status = FIRE_STATUS::OUT_OF_AMMO;
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
            glUniform3f(uLightColorLoc, 255.0/255.0, 51.0/255.0, 51.0/255.0);
        }
        else {
            glUniform3f(uLightColorLoc, 0.2, 0.2, 0.2);
        }

        glUseProgram(moveShader);
        glUniform2f(uOffsetLoc, offset_x, offset_y);

        if (!view_in_cockpit) {

            unsigned int targetVAOs[3] = {VAO[3], VAO[4], VAO[5] };

            drawLandscape(moveShader, VAO[0], landscapeTexture);
            drawTargets(moveShader, targetVAOs , targetTexture, targets_hit);
            drawSight(unifiedShader, VAO[0], sightTexture);
        }
        else {
            drawLandscape(moveShader, VAO[0], cockpitTexture);
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
    glDeleteBuffers(1, &VBO);
    glDeleteVertexArrays(2, VAO);
    glDeleteProgram(unifiedShader);

    glfwTerminate();
    return 0;
}

bool is_target_hit(float offset_x, float offset_y, float *target) {
    float x_min = target[0];
    float y_min = target[1];
    float x_max = target[8];
    float y_max = target[9];

    return ((-offset_x) > x_min && (-offset_x) < x_max && (- offset_y) > y_min && (-offset_y) < y_max );
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

void handleMove(GLFWwindow* window,bool hidraulic_on, bool view_in_cockpit, float default_movement_speed, float min_voltmeter_value, float voltmeter_value, float *offset_x, float max_offset_x, float *offset_y, float max_offset_y) {
    float movement_speed;
    if (!hidraulic_on) {
        movement_speed = default_movement_speed / 10;
    }
    else {
        movement_speed = default_movement_speed + ((abs(min_voltmeter_value) + voltmeter_value) / 500);
    }

    if (glfwGetKey(window, GLFW_KEY_LEFT) == GLFW_PRESS) {
        if (!view_in_cockpit) {
            *offset_x += movement_speed;
        }
        if (*offset_x >= max_offset_x) {
            *offset_x = max_offset_x;
        }
    }
    if (glfwGetKey(window, GLFW_KEY_RIGHT) == GLFW_PRESS) {
        if (!view_in_cockpit) {
            *offset_x -= movement_speed;
        }
        if (*offset_x <= -max_offset_x) {
            *offset_x = -max_offset_x;
        }
    }
    if (glfwGetKey(window, GLFW_KEY_DOWN) == GLFW_PRESS) {
        if (!view_in_cockpit) {
            *offset_y += movement_speed;
        }
        if (*offset_y >= max_offset_y) {
            *offset_y = max_offset_y;
        }
    }
    if (glfwGetKey(window, GLFW_KEY_UP) == GLFW_PRESS) {
        if (!view_in_cockpit) {
            *offset_y -= movement_speed;
        }
        if (*offset_y <= -max_offset_y) {
            *offset_y = -max_offset_y;
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