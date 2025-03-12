#include <stdio.h>
#include <stdlib.h>
#include <GL/freeglut.h>
#include <math.h>

// 旋轉角度
float angleX = 0.0f, angleY = 0.0f, angleZ = 0.0f;
float posX = 0.0f, posY = 0.0f, posZ = 0.0f;
float scale = 1.0f;

void drawAxes() {
    glLineWidth(2.0f);
    glBegin(GL_LINES);

    // X 軸 (紅色)
    glColor3f(1, 0, 0);
    glVertex3f(-2, 0, 0);
    glVertex3f(2, 0, 0);

    // Y 軸 (綠色)
    glColor3f(0, 1, 0);
    glVertex3f(0, -2, 0);
    glVertex3f(0, 2, 0);

    // Z 軸 (藍色)
    glColor3f(0, 0, 1);
    glVertex3f(0, 0, -2);
    glVertex3f(0, 0, 2);

    glEnd();
    glLineWidth(1.0f);
}

void drawCube() {
    glPushMatrix();
    glTranslatef(posX, posY, posZ);
    glRotatef(angleX, 1, 0, 0);
    glRotatef(angleY, 0, 1, 0);
    glRotatef(angleZ, 0, 0, 1);
    glScalef(scale, scale, scale);

    glBegin(GL_QUADS);

    // 顏色與面對應
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

void keyboard(unsigned char key, int x, int y) {
    switch (key) {
    case 'w': posY += 0.1f; break;
    case 's': posY -= 0.1f; break;
    case 'a': posX -= 0.1f; break;
    case 'd': posX += 0.1f; break;
    case 'q': posZ += 0.1f; break;
    case 'e': posZ -= 0.1f; break;
    case 'i': angleX += 5.0f; break;
    case 'k': angleX -= 5.0f; break;
    case 'j': angleY += 5.0f; break;
    case 'l': angleY -= 5.0f; break;
    case 'u': angleZ += 5.0f; break;
    case 'o': angleZ -= 5.0f; break;
    case '+': scale *= 1.1f; break;
    case '-': scale /= 1.1f; break;
    case 'r': // 重置
        posX = posY = posZ = 0.0f;
        angleX = angleY = angleZ = 0.0f;
        scale = 1.0f;
        break;
    case 27: exit(0);
    }
    glutPostRedisplay();
}

void display() {
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glLoadIdentity();
    gluLookAt(3, 3, 3, 0, 0, 0, 0, 1, 0);
    drawAxes();
    drawCube();
    glutSwapBuffers();
}

void reshape(int w, int h) {
    glViewport(0, 0, w, h);
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    gluPerspective(45.0, (float)w / (float)h, 1.0, 10.0);
    glMatrixMode(GL_MODELVIEW);
}

int main(int argc, char** argv) {
    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB | GLUT_DEPTH);
    glutInitWindowSize(800, 600);
    glutCreateWindow("FreeGLUT 3D Cube");

    glEnable(GL_DEPTH_TEST);

    glutDisplayFunc(display);
    glutReshapeFunc(reshape);
    glutKeyboardFunc(keyboard);

    glutMainLoop();
    return 0;
}