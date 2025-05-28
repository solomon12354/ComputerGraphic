#include <stdio.h>
#include <stdlib.h>
#include <GL/freeglut.h>
#include <math.h>

// 定義 PI，如果沒有的話
#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

// 旋轉角度
float angle = 0.0f;
float posX = 0.0f, posY = 0.0f, posZ = 0.0f;
float scale = 1.0f;

// 旋轉軸
float axisX = 0.0f, axisY = 1.0f, axisZ = 0.0f;

// 計算旋轉矩陣 (Rodrigues' Rotation Formula)
void applyRotation(float angle) {
    float rad = angle * M_PI / 180.0f;
    float c = cos(rad);
    float s = sin(rad);
    float t = 1 - c;

    float rotationMatrix[16] = {
        t * axisX * axisX + c,         t * axisX * axisY - s * axisZ, t * axisX * axisZ + s * axisY, 0,
        t * axisX * axisY + s * axisZ, t * axisY * axisY + c,         t * axisY * axisZ - s * axisX, 0,
        t * axisX * axisZ - s * axisY, t * axisY * axisZ + s * axisX, t * axisZ * axisZ + c,         0,
        0, 0, 0, 1
    };

    glMultMatrixf(rotationMatrix);
}

// 繪製立方體
void drawCube() {
    glPushMatrix();
    glTranslatef(posX, posY, posZ);
    applyRotation(angle); // 應用旋轉矩陣
    glScalef(scale, scale, scale);

    glBegin(GL_QUADS);

    glColor3f(1, 0, 0); // 紅色 - 前面
    glVertex3f(-0.5, -0.5, 0.5);
    glVertex3f(0.5, -0.5, 0.5);
    glVertex3f(0.5, 0.5, 0.5);
    glVertex3f(-0.5, 0.5, 0.5);

    glColor3f(0, 1, 0); // 綠色 - 後面
    glVertex3f(-0.5, -0.5, -0.5);
    glVertex3f(-0.5, 0.5, -0.5);
    glVertex3f(0.5, 0.5, -0.5);
    glVertex3f(0.5, -0.5, -0.5);

    glColor3f(0, 0, 1); // 藍色 - 左側
    glVertex3f(-0.5, -0.5, -0.5);
    glVertex3f(-0.5, -0.5, 0.5);
    glVertex3f(-0.5, 0.5, 0.5);
    glVertex3f(-0.5, 0.5, -0.5);

    glColor3f(1, 1, 0); // 黃色 - 右側
    glVertex3f(0.5, -0.5, -0.5);
    glVertex3f(0.5, 0.5, -0.5);
    glVertex3f(0.5, 0.5, 0.5);
    glVertex3f(0.5, -0.5, 0.5);

    glColor3f(1, 0, 1); // 紫色 - 上面
    glVertex3f(-0.5, 0.5, -0.5);
    glVertex3f(-0.5, 0.5, 0.5);
    glVertex3f(0.5, 0.5, 0.5);
    glVertex3f(0.5, 0.5, -0.5);

    glColor3f(0, 1, 1); // 青色 - 下面
    glVertex3f(-0.5, -0.5, -0.5);
    glVertex3f(0.5, -0.5, -0.5);
    glVertex3f(0.5, -0.5, 0.5);
    glVertex3f(-0.5, -0.5, 0.5);

    glEnd();
    glPopMatrix();
}

// 鍵盤控制
void keyboard(unsigned char key, int x, int y) {
    switch (key) {
    case 'w': posY += 0.1f; break;
    case 's': posY -= 0.1f; break;
    case 'a': posX -= 0.1f; break;
    case 'd': posX += 0.1f; break;
    case 'q': posZ += 0.1f; break;
    case 'e': posZ -= 0.1f; break;
    case 'r': angle += 5.0f; break;
    case 'o': // 重置
        posX = posY = posZ = 0.0f;
        angle = 0.0f;
        scale = 1.0f;
        axisX = 0.0f, axisY = 1.0f, axisZ = 0.0f;
        break;
    case '1': axisX = 1.0f, axisY = 0.0f, axisZ = 0.0f; break;
    case '2': axisX = 0.0f, axisY = 1.0f, axisZ = 0.0f; break;
    case '3': axisX = 0.0f, axisY = 0.0f, axisZ = 1.0f; break;
    case '+': scale *= 1.1f; break;
    case '-': scale /= 1.1f; break;
    case 27: exit(0);
    }
    glutPostRedisplay();
}

// 顯示函數
void display() {
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glLoadIdentity();
    gluLookAt(3, 3, 3, 0, 0, 0, 0, 1, 0);
    drawCube();
    glutSwapBuffers();
}

// 視窗變換
void reshape(int w, int h) {
    glViewport(0, 0, w, h);
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    gluPerspective(45.0, (float)w / (float)h, 1.0, 10.0);
    glMatrixMode(GL_MODELVIEW);
}

// 主函數
int main(int argc, char** argv) {
    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB | GLUT_DEPTH);
    glutInitWindowSize(800, 600);
    glutCreateWindow("Arbitrary Axis Rotation");

    glEnable(GL_DEPTH_TEST);
    glutDisplayFunc(display);
    glutReshapeFunc(reshape);
    glutKeyboardFunc(keyboard);

    glutMainLoop();
    return 0;
}
