#include <opencv2/opencv.hpp>
#include <GL/freeglut.h>

using namespace cv;

GLuint textures[4];  // 0: wood, 1: S, 2: G, 3: 5
float angleX = 30.0f, angleY = 30.0f;
int showStage = 0;

void loadTexture(Mat& img, GLuint textureID) {
    glBindTexture(GL_TEXTURE_2D, textureID);
    cvtColor(img, img, COLOR_BGR2RGB);
    flip(img, img, 0);  // OpenGL 座標上下顛倒

    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, img.cols, img.rows, 0,
        GL_RGB, GL_UNSIGNED_BYTE, img.data);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
}

void timer(int value) {
    if (showStage < 5) {
        showStage++;
        glutPostRedisplay();
        glutTimerFunc(1000, timer, 0);  // 每 1 秒更新一次
    }
}

void drawCube() {
    glEnable(GL_TEXTURE_2D);

    // 底部 wood
    if (showStage >= 1) {
        glBindTexture(GL_TEXTURE_2D, textures[0]);
        glBegin(GL_QUADS);
        glTexCoord2f(0, 0); glVertex3f(-2, -1, 2);
        glTexCoord2f(1, 0); glVertex3f(2, -1, 2);
        glTexCoord2f(1, 1); glVertex3f(2, -1, -2);
        glTexCoord2f(0, 1); glVertex3f(-2, -1, -2);
        glEnd();
    }

    // 前面 S
    if (showStage >= 2) {
        glBindTexture(GL_TEXTURE_2D, textures[1]);
        glBegin(GL_QUADS);
        glTexCoord2f(0, 0); glVertex3f(-1, -1, 1);
        glTexCoord2f(1, 0); glVertex3f(1, -1, 1);
        glTexCoord2f(1, 1); glVertex3f(1, 1, 1);
        glTexCoord2f(0, 1); glVertex3f(-1, 1, 1);
        glEnd();
    }

    // 右側 G
    if (showStage >= 3) {
        glBindTexture(GL_TEXTURE_2D, textures[2]);
        glBegin(GL_QUADS);
        glTexCoord2f(0, 0); glVertex3f(1, -1, 1);
        glTexCoord2f(1, 0); glVertex3f(1, -1, -1);
        glTexCoord2f(1, 1); glVertex3f(1, 1, -1);
        glTexCoord2f(0, 1); glVertex3f(1, 1, 1);
        glEnd();
    }

    // 上面 5
    if (showStage >= 4) {
        glBindTexture(GL_TEXTURE_2D, textures[3]);
        glBegin(GL_QUADS);
        glTexCoord2f(0, 0); glVertex3f(-1, 1, 1);
        glTexCoord2f(1, 0); glVertex3f(1, 1, 1);
        glTexCoord2f(1, 1); glVertex3f(1, 1, -1);
        glTexCoord2f(0, 1); glVertex3f(-1, 1, -1);
        glEnd();
    }
    

    glDisable(GL_TEXTURE_2D);
}

void display() {
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    glPushMatrix();
    glRotatef(angleX, 1.0, 0.0, 0.0);
    glRotatef(angleY, 0.0, -1.0, 0.0);

    drawCube();

    glPopMatrix();
    glutSwapBuffers();
}

void idle() {
    //angleY += 0.3f;
    if (angleY > 360) angleY -= 360;
    glutPostRedisplay();
}

void reshape(int w, int h) {
    glViewport(0, 0, w, h);
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    gluPerspective(45, (float)w / h, 1, 10);
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
    gluLookAt(0, 0, 5, 0, 0, 0, 0, 1, 0);
}

int main(int argc, char** argv) {
    // 讀取貼圖圖片
    Mat wood = imread("C:\\Users\\User\\Downloads\\OpenCVTexture\\floor.jpg");
    Mat imgS = imread("C:\\Users\\User\\Downloads\\OpenCVTexture\\Block6.jpg");
    Mat imgG = imread("C:\\Users\\User\\Downloads\\OpenCVTexture\\Block5.jpg");
    Mat img5 = imread("C:\\Users\\User\\Downloads\\OpenCVTexture\\Block4.jpg");

    if (wood.empty() || imgS.empty() || imgG.empty() || img5.empty()) {
        printf("請確認圖片 wood.jpg, S.jpg, G.jpg, 5.jpg 都存在！\n");
        return -1;
    }

    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB | GLUT_DEPTH);
    glutInitWindowSize(800, 600);
    glutCreateWindow("OpenCV + FreeGLUT Cube");

    glEnable(GL_DEPTH_TEST);
    glGenTextures(4, textures);

    loadTexture(wood, textures[0]);
    loadTexture(imgS, textures[1]);
    loadTexture(imgG, textures[2]);
    loadTexture(img5, textures[3]);

    glutDisplayFunc(display);
    glutReshapeFunc(reshape);
    glutIdleFunc(idle);

    glutTimerFunc(1000, timer, 0);

    glutMainLoop();

    return 0;
}
