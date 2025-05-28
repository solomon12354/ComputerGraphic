#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <vector>
#include <GL/freeglut.h>
#include <math.h>
#include <time.h>

typedef struct {
    float x, y, z;
} Vec3;

typedef struct {
    int v1, v2, v3;
} Face;

std::vector<Vec3> vertices;
std::vector<Face> faces;

float rotateX = 0.0f;
float rotateY = 0.0f;
float rotateZ = 0.0f;

Vec3 colors = { 0.8f, 0.5f, 0.2f };

Vec3 modelCenter = { 0, 0, 0 };
float modelScale = 1.0f;

float mag = 1.0f;

GLenum mode = GL_LINES;

void loadOBJ(const char* filename) {
    FILE* file = fopen(filename, "r");
    if (!file) {
        printf("無法打開檔案: %s\n", filename);
        exit(1);
    }

    char line[128];
    while (fgets(line, sizeof(line), file)) {
        if (strncmp(line, "v ", 2) == 0) {
            Vec3 v;
            sscanf(line, "v %f %f %f", &v.x, &v.y, &v.z);
            printf("v %f %f %f\n", v.x, v.y, v.z);

            vertices.push_back(v);
        }
        else if (strncmp(line, "f ", 2) == 0) {
            Face f;
            int v1, v2, v3;
            if (sscanf(line, "f %d/%*d/%*d %d/%*d/%*d %d/%*d/%*d", &v1, &v2, &v3) == 3 ||
                sscanf(line, "f %d %d %d", &v1, &v2, &v3) == 3) {
                f.v1 = v1 - 1;
                f.v2 = v2 - 1;
                f.v3 = v3 - 1;
                faces.push_back(f);
                printf("f %d %d %d\n", f.v1, f.v2, f.v3);
            }
        }
    }
    printf("Open File %s successfully.\n", filename);
    fclose(file);
}

float cameraDistance = 3.0f;  // 預設距離，自動調整

void computeCenterAndScale() {
    if (vertices.empty()) return;

    Vec3 min = vertices[0];
    Vec3 max = vertices[0];

    for (const Vec3& v : vertices) {
        if (v.x < min.x) min.x = v.x;
        if (v.y < min.y) min.y = v.y;
        if (v.z < min.z) min.z = v.z;
        if (v.x > max.x) max.x = v.x;
        if (v.y > max.y) max.y = v.y;
        if (v.z > max.z) max.z = v.z;
    }

    modelCenter.x = (min.x + max.x) / 2.0f;
    modelCenter.y = (min.y + max.y) / 2.0f;
    modelCenter.z = (min.z + max.z) / 2.0f;

    float dx = max.x - min.x;
    float dy = max.y - min.y;
    float dz = max.z - min.z;
    float maxDim = fmaxf(fmaxf(dx, dy), dz);

    // 自動縮放模型，使其佔螢幕 75%
    float screenCoverageRatio = 0.75f;

    // 視野角度為 45 度（與 gluPerspective 相符）
    float fovY = 45.0f * (3.14159f / 180.0f); // 轉為 radians
    float viewHeight = 2.0f * tanf(fovY / 2.0f) * 3.0f; // z = -3.0f
    modelScale = (viewHeight * screenCoverageRatio) / maxDim;
}





void display() {
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    gluPerspective(60.0, 800.0 / 600.0, 0.1, 100.0);

    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
    glTranslatef(0.0f, 0.0f, -3.0f);

    // 依序繞 Z、Y、X 軸旋轉
    glRotatef(rotateZ, 0.0f, 0.0f, 1.0f);
    glRotatef(rotateY, 0.0f, 1.0f, 0.0f);
    glRotatef(rotateX, 1.0f, 0.0f, 0.0f);

    glScalef(modelScale * mag, modelScale * mag, modelScale * mag);
    glTranslatef(-modelCenter.x, -modelCenter.y, -modelCenter.z);


    glColor3f(colors.x, colors.y, colors.z);

    glBegin(mode);
    for (size_t i = 0; i < faces.size(); ++i) {
        Vec3 v1 = vertices[faces[i].v1];
        Vec3 v2 = vertices[faces[i].v2];
        Vec3 v3 = vertices[faces[i].v3];
        glVertex3f(v1.x, v1.y, v1.z);
        glVertex3f(v2.x, v2.y, v2.z);
        glVertex3f(v3.x, v3.y, v3.z);
    }
    glEnd();
    if (mode == GL_TRIANGLES) {
        glDisable(GL_LIGHTING); // 線條不受光影影響
        glColor3f(0.0f, 0.0f, 0.0f); // 黑色邊線
        glLineWidth(1.0f); // 可以加粗
        for (const Face& f : faces) {
            Vec3 v1 = vertices[f.v1];
            Vec3 v2 = vertices[f.v2];
            Vec3 v3 = vertices[f.v3];

            glBegin(GL_LINE_LOOP);
            glVertex3f(v1.x, v1.y, v1.z);
            glVertex3f(v2.x, v2.y, v2.z);
            glVertex3f(v3.x, v3.y, v3.z);
            glEnd();
        }
    }
    glutSwapBuffers();
}

float moveMag = 0.5f;

void keyboard(unsigned char key, int x, int y) {
    switch (key) {
    case 'x':
        rotateX += (5.0f);
        break;
    case 'X':
        rotateX -= (5.0f);
        break;
    case 'y':
        rotateY += (5.0f);
        break;
    case 'Y':
        rotateY -= (5.0f);
        break;
    case 'z':
        rotateZ += (5.0f);
        break;
    case 'Z':
        rotateZ -= (5.0f);
        break;
    case 'w':
        modelCenter.y -= (5.0f * moveMag);
        break;
    case 's':
        modelCenter.y += (5.0f * moveMag);
        break;
    case 'a':
        modelCenter.x += (5.0f * moveMag);
        break;
    case 'd':
        modelCenter.x -= (5.0f * moveMag);
        break;
    case 'q':
        modelCenter.z += (5.0f * moveMag);
        break;
    case 'e':
        modelCenter.z -= (5.0f * moveMag);
        break;
    case 'W':
        modelCenter.y -= (5.0f * moveMag);
        break;
    case 'S':
        modelCenter.y += (5.0f * moveMag);
        break;
    case 'A':
        modelCenter.x += (5.0f * moveMag);
        break;
    case 'D':
        modelCenter.x -= (5.0f * moveMag);
        break;
    case 'Q':
        modelCenter.z += (5.0f * moveMag);
        break;
    case 'E':
        modelCenter.z -= (5.0f * moveMag);
        break;
    case 27:  // ESC 鍵退出
        exit(0);
        break;
    }
    glutPostRedisplay();  // 重新繪製
}

void Menu(int index) {
    switch (index) {
    case 1:
        vertices.clear();
        faces.clear();
        rotateX = 0.0f;
        rotateY = 0.0f;
        rotateZ = 0.0f;
        modelCenter = { 0, 0, 0 };
        mag = 0.3f;
        moveMag = 0.5f;
        colors.x = 0.8f;
        colors.y = 0.5f;
        colors.z = 0.2f;
        loadOBJ("C:/Users/User/Downloads/OBJs4/teapot.obj");
        break;
    case 2:
        vertices.clear();
        faces.clear();
        rotateX = 0.0f;
        rotateY = 0.0f;
        rotateZ = 0.0f;
        modelCenter = { 0, 0, 0 };
        mag = 7.3f;
        moveMag = 0.05f;
        colors.x = 0.8f;
        colors.y = 0.5f;
        colors.z = 0.2f;
        loadOBJ("C:/Users/User/Downloads/OBJs4/gourd.obj");
        break;
    case 3:
        vertices.clear();
        faces.clear();
        rotateX = 0.0f;
        rotateY = 0.0f;
        rotateZ = 0.0f;
        modelCenter = { 0, 0, 0 };
        mag = 9.0f;
        moveMag = 0.05f;
        colors.x = 0.8f;
        colors.y = 0.5f;
        colors.z = 0.2f;
        loadOBJ("C:/Users/User/Downloads/OBJs4/octahedron.obj");
        break;
    case 4:
        vertices.clear();
        faces.clear();
        rotateX = 0.0f;
        rotateY = 0.0f;
        rotateZ = 0.0f;
        modelCenter = { 0, 0, 0 };
        mag = 1.0f;
        moveMag = 0.5f;
        colors.x = 0.8f;
        colors.y = 0.5f;
        colors.z = 0.2f;
        loadOBJ("C:/Users/User/Downloads/OBJs4/teddy.obj");
        break;
    case 5:
        colors.x = 0.8f;
        colors.y = 0.5f;
        colors.z = 0.2f;
        break;
    case 6:
        srand(time(NULL));



        colors.x = (float)rand() / (RAND_MAX + 1.0);
        colors.y = (float)rand() / (RAND_MAX + 1.0);
        colors.z = (float)rand() / (RAND_MAX + 1.0);
        break;
    case 7:
        mode = GL_POINTS;
        break;
    case 8:
        mode = GL_LINES;
        break;
    case 9:
        mode = GL_TRIANGLES;
        break;
    }
    glutPostRedisplay();
}

void menu2(int index) {
    switch (index) {

    case 5:
        colors.x = 0.8f;
        colors.y = 0.5f;
        colors.z = 0.2f;
        break;
    case 6:
        srand(time(NULL));



        colors.x = (float)rand() / (RAND_MAX + 1.0);
        colors.y = (float)rand() / (RAND_MAX + 1.0);
        colors.z = (float)rand() / (RAND_MAX + 1.0);
        break;

    }
    glutPostRedisplay();
}

void menu3(int index) {
    switch (index) {

    case 7:
        mode = GL_POINTS;
        break;
    case 8:
        mode = GL_LINES;
        break;
    case 9:
        mode = GL_TRIANGLES;
        break;

    }
    glutPostRedisplay();
}

void idle() {
    rotateY += 0.2f;
    if (rotateY > 360.0f) rotateY -= 360.0f;
    glutPostRedisplay();
}

void initOpenGL() {
    glEnable(GL_DEPTH_TEST);
    glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
}

int main(int argc, char** argv) {
    loadOBJ("C:/Users/User/Downloads/OBJs4/teddy.obj");
    computeCenterAndScale();

    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB | GLUT_DEPTH);
    glutInitWindowSize(800, 600);
    glutCreateWindow("Teapot OBJ Viewer (FreeGLUT)");

    initOpenGL();
    int renderMenu = glutCreateMenu(Menu);
    glutAddMenuEntry("teapot", 1);
    glutAddMenuEntry("gourd", 2);
    glutAddMenuEntry("octahedron", 3);
    glutAddMenuEntry("teddy", 4);
    //glutCreateMenu(Menu);
    //glutAddSubMenu("Object Menu", renderMenu);
    int colorMenu = glutCreateMenu(Menu);
    glutAddMenuEntry("singleColor", 5);
    glutAddMenuEntry("randomColor", 6);
    //glutCreateMenu(menu2);
    //glutAddSubMenu("Color Menu", colorMenu);

    int modeMenu = glutCreateMenu(Menu);
    glutAddMenuEntry("point", 7);
    glutAddMenuEntry("line", 8);
    glutAddMenuEntry("triangle", 9);

    //glutCreateMenu(menu2);
    int mainMenu = glutCreateMenu(Menu);
    glutAddSubMenu("Object Menu", renderMenu);
    glutAddSubMenu("Color Menu", colorMenu);
    glutAddSubMenu("Mode Menu", modeMenu);

    glutAttachMenu(GLUT_RIGHT_BUTTON);
    glutDisplayFunc(display);
    //glutIdleFunc(idle);
    glutKeyboardFunc(keyboard);
    glutMainLoop();
    return 0;
}
