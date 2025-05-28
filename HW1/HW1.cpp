#include <stdio.h>
#include <stdlib.h>
#include <GL/freeglut.h>
#include <math.h>

#define SMOOTH 1
#define FLAT 2
#define SIDES 6  // 六邊形的邊數
#define PI 3.14159265

void ChangeSize(int, int);
void RenderScene(void);
void Menu(int);

int _mode = GL_SMOOTH;

int main(int argc, char** argv) {
    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB);
    glutInitWindowSize(400, 400);
    glutInitWindowPosition(600, 80);
    glutCreateWindow("Hexagon - Two Colors Gradient");

    glutCreateMenu(Menu);
    glutAddMenuEntry("Smooth", SMOOTH);
    glutAddMenuEntry("Flat (Two Colors Gradient)", FLAT);
    glutAttachMenu(GLUT_RIGHT_BUTTON);

    glutReshapeFunc(ChangeSize);
    glutDisplayFunc(RenderScene);
    glutPostRedisplay();
    glutMainLoop();
    return 0;
}

void ChangeSize(int w, int h) {
    printf("Window Size = %d x %d\n", w, h);
    glViewport(0, 0, w, h);
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    glOrtho(-6, 6, -6, 6, -1, 1);  // 視野範圍
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
}

void Menu(int index) {
    switch (index) {
    case SMOOTH:
        _mode = GL_SMOOTH;
        break;
    case FLAT:
        _mode = GL_FLAT;
        break;
    }
    glutPostRedisplay();
}

void RenderScene(void) {
    glClearColor(1.0, 1.0, 1.0, 1.0);
    glClear(GL_COLOR_BUFFER_BIT);
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();

    float radius = 5.0f;
    float centerX = 0, centerY = 0; // 中心點

    // **只使用兩種顏色**
    float colorA[3] = { 1, 0, 0 };  // 紅色
    float colorB[3] = { 0, 0, 1 };  // 藍色

    if (_mode == GL_SMOOTH) {
        // **SMOOTH 模式：六個頂點不同顏色**
        float colors[SIDES][3] = {
            {1, 0, 0},  // 紅色
            {0, 1, 0},  // 綠色
            {0, 0, 1},  // 藍色
            {1, 1, 0},  // 黃色
            {1, 0, 1},  // 紫色
            {0, 1, 1}   // 青色
        };

        glBegin(GL_POLYGON);
        for (int i = 0; i < SIDES; i++) {
            float angle = 2.0f * PI * i / SIDES;
            float x = radius * cos(angle);
            float y = radius * sin(angle);
            glColor3f(colors[i][0], colors[i][1], colors[i][2]);
            glVertex2f(x, y);
        }
        glEnd();
    }
    else if (_mode == GL_FLAT) {
        // **FLAT 模式：使用兩種顏色，但有漸變**
        glBegin(GL_POLYGON);
        for (int i = 0; i < SIDES; i++) {
            float angle = 2.0f * PI * i / SIDES;
            float x = radius * cos(angle);
            float y = radius * sin(angle);

            // **交錯使用兩種顏色，但仍然讓 OpenGL 進行插值**
            if (i % 2 == 0)
                glColor3f(colorA[0], colorA[1], colorA[2]);  // 紅色
            else
                glColor3f(colorB[0], colorB[1], colorB[2]);  // 藍色

            glVertex2f(x, y);
        }
        glEnd();
    }

    glutSwapBuffers();
}
