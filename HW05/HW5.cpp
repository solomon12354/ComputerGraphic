#include <stdio.h>
#include <stdlib.h>
#include <GL/freeglut.h>

#include <math.h>

#define M_PI 3.14159265358979323846

bool isT = false;
bool hasPoint = false;

#define SMOOTH 1
#define FLAT 2

int _mode = 0;

float pointX, pointY, pointZ;

float vertices[8][3] = {
    {-0.5, -0.5, 0.5},
    {0.5, -0.5, 0.5},
    {0.5, 0.5, 0.5},
    {-0.5, 0.5, 0.5},
    {-0.5, -0.5, -0.5},
    {0.5, -0.5, -0.5},
    {0.5, 0.5, -0.5},
    {-0.5, 0.5, -0.5}
};

// 旋轉角度
float angleX = 0.0f, angleY = 0.0f, angleZ = 0.0f;
float posX = 0.0f, posY = 0.0f, posZ = 0.0f;
float scale = 1.0f;
float selfRotationAngle = 0.0f;

// 定義法向量，這是物體的初始法向量
float normalX = 0.0f;
float normalY = 1.0f;  // 假設最初指向Y軸
float normalZ = 0.0f;

// 兩個座標 v1 和 v2
float v1[3] = {0.0f, 0.0f, 0.0f};
float v2[3] = {3.0f, 2.0f, 1.0f};  // 預設的 v2 座標

// 自訂平移函式
void translate(float* x, float* y, float* z, float posX, float posY, float posZ) {
    *x += posX;
    *y += posY;
    *z += posZ;
}

// 自訂繞X軸旋轉函式
void rotateX(float* x, float* y, float* z, float angle) {
    float radian = angle * (M_PI / 180.0f);  // 角度轉弧度
    float cosA = cos(radian);
    float sinA = sin(radian);

    float newY = (*y) * cosA - (*z) * sinA;
    float newZ = (*y) * sinA + (*z) * cosA;

    *y = newY;
    *z = newZ;
}



// 自訂繞Y軸旋轉函式
void rotateY(float* x, float* y, float* z, float angle) {
    float radian = angle * (M_PI / 180.0f);  // 角度轉弧度
    float cosA = cos(radian);
    float sinA = sin(radian);

    float newX = (*x) * cosA + (*z) * sinA;
    float newZ = -(*x) * sinA + (*z) * cosA;

    *x = newX;
    *z = newZ;
}

// 自訂繞Z軸旋轉函式
void rotateZ(float* x, float* y, float* z, float angle) {
    float radian = angle * (M_PI / 180.0f);  // 角度轉弧度
    float cosA = cos(radian);
    float sinA = sin(radian);

    float newX = (*x) * cosA - (*y) * sinA;
    float newY = (*x) * sinA + (*y) * cosA;

    *x = newX;
    *y = newY;
}

void get3DCoordinates(int x, int y, float& outX, float& outY, float& outZ) {
    GLint viewport[4];
    GLdouble modelview[16], projection[16];
    GLfloat winX, winY, winZ;
    GLdouble worldX, worldY, worldZ;

    glGetIntegerv(GL_VIEWPORT, viewport);
    glGetDoublev(GL_MODELVIEW_MATRIX, modelview);
    glGetDoublev(GL_PROJECTION_MATRIX, projection);

    winX = (float)x;
    winY = (float)(viewport[3] - y); // 轉換 Y 軸
    glReadPixels(x, int(winY), 1, 1, GL_DEPTH_COMPONENT, GL_FLOAT, &winZ); // 讀取 Z 深度

    gluUnProject(winX, winY, winZ, modelview, projection, viewport, &worldX, &worldY, &worldZ);

    outX = (float)worldX;
    outY = (float)worldY;
    outZ = (float)worldZ;
}

void mouseClick(int button, int state, int x, int y) {
    if (button == GLUT_LEFT_BUTTON && state == GLUT_DOWN) {
        get3DCoordinates(x, y, pointX, pointY, pointZ);
        hasPoint = true;
        //cout << "Clicked: (" << pointX << ", " << pointY << ", " << pointZ << ")" << endl;
        glutPostRedisplay();
    }
}

void rotateAroundVector(float* x, float* y, float* z, float angle,int method) {
    float dx = 0;
    float dy = 0;
    float dz = 0;
    if (method == 0) {
        dx = v2[0] - v1[0];
        dy = v2[1] - v1[1];
        dz = v2[2] - v1[2];
    }else if (method == 1) {
        dx = pointX;
        dy = pointY;
        dz = pointZ;
    }


    float radian = angle * (M_PI / 180.0f);
    float cosA = cos(radian);
    float sinA = sin(radian);

    // 計算新座標
    float newY = (*y) * cosA - (*z) * sinA;
    float newZ = (*y) * sinA + (*z) * cosA;

    // 更新座標
    *y = newY;
    *z = newZ;
}

// 顯示座標軸
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

// 繪製連線
void drawLine() {
    glBegin(GL_LINES);
    glColor3f(1, 1, 0);  // 黃色
    glVertex3fv(v1);
    glVertex3fv(v2);
    glEnd();
}

void myglTranslatef(GLfloat x, GLfloat y, GLfloat z) {
    GLfloat translationMatrix[16] = {
        1, 0, 0, x,  // 第一列
        0, 1, 0, y,  // 第二列
        0, 0, 1, z,  // 第三列
        0, 0, 0, 1   // 第四列 (齊次座標)
    };

    // 取得當前矩陣並與平移矩陣相乘
    glMultMatrixf(translationMatrix);
}



void myglRotatef(float angle, float x, float y, float z) {
    float radian = angle * 3.14159265f / 180.0f;
    float c = cos(radian);
    float s = sin(radian);
    float matrix[16] = {
        x * x * (1 - c) + c,     x * y * (1 - c) - z * s, x * z * (1 - c) + y * s, 0,
        y * x * (1 - c) + z * s, y * y * (1 - c) + c,     y * z * (1 - c) - x * s, 0,
        z * x * (1 - c) - y * s, z * y * (1 - c) + x * s, z * z * (1 - c) + c,     0,
        0,                       0,                       0,                       1
    };
    glMultMatrixf(matrix);
}

void drawCube() {
    glPushMatrix();

    // 手動平移
    float vertices[8][3] = {
        {-0.5f, -0.5f, 0.5f},  // 0
        { 0.5f, -0.5f, 0.5f},  // 1
        { 0.5f,  0.5f, 0.5f},  // 2
        {-0.5f,  0.5f, 0.5f},  // 3
        {-0.5f, -0.5f, -0.5f}, // 4
        { 0.5f, -0.5f, -0.5f}, // 5
        { 0.5f,  0.5f, -0.5f}, // 6
        {-0.5f,  0.5f, -0.5f}  // 7
    };

    // 手動平移所有頂點
    for (int i = 0; i < 8; i++) {
        translate(&vertices[i][0], &vertices[i][1], &vertices[i][2], posX, posY, posZ);
    }

    // 手動旋轉所有頂點
    for (int i = 0; i < 8; i++) {
        rotateX(&vertices[i][0], &vertices[i][1], &vertices[i][2], angleX);
        rotateY(&vertices[i][0], &vertices[i][1], &vertices[i][2], angleY);
        rotateZ(&vertices[i][0], &vertices[i][1], &vertices[i][2], angleZ);
    }
    


    glBegin(GL_QUADS);

    // 顏色與面對應
    glColor3f(1, 0, 0); // 紅色 - 前面
    glVertex3fv(vertices[0]);
    glVertex3fv(vertices[1]);
    glVertex3fv(vertices[2]);
    glVertex3fv(vertices[3]);

    glColor3f(0, 1, 0); // 綠色 - 後面
    glVertex3fv(vertices[4]);
    glVertex3fv(vertices[5]);
    glVertex3fv(vertices[6]);
    glVertex3fv(vertices[7]);

    glColor3f(0, 0, 1); // 藍色 - 左側
    glVertex3fv(vertices[4]);
    glVertex3fv(vertices[0]);
    glVertex3fv(vertices[3]);
    glVertex3fv(vertices[7]);

    glColor3f(1, 1, 0); // 黃色 - 右側
    glVertex3fv(vertices[1]);
    glVertex3fv(vertices[5]);
    glVertex3fv(vertices[6]);
    glVertex3fv(vertices[2]);

    glColor3f(1, 0, 1); // 紫色 - 上面
    glVertex3fv(vertices[3]);
    glVertex3fv(vertices[2]);
    glVertex3fv(vertices[6]);
    glVertex3fv(vertices[7]);

    glColor3f(0, 1, 1); // 青色 - 下面
    glVertex3fv(vertices[0]);
    glVertex3fv(vertices[1]);
    glVertex3fv(vertices[5]);
    glVertex3fv(vertices[4]);

    glEnd();

    glPopMatrix();
}

// 繪製方塊



void calculateVectorBetweenV1AndV2(float* dx, float* dy, float* dz) {
    *dx = v2[0] - v1[0];
    *dy = v2[1] - v1[1];
    *dz = v2[2] - v1[2];
}

void computeRotationAxis() {
    float dx = v2[0] - v1[0];
    float dy = v2[1] - v1[1];
    float dz = v2[2] - v1[2];
    float length = sqrt(dx * dx + dy * dy + dz * dz);
    if (length == 0) return;
    angleX = dx / length;
    angleY = dy / length;
    angleZ = dz / length;
}

// 計算兩個向量的點積
float dotProduct(float x1, float y1, float z1, float x2, float y2, float z2) {
    return x1 * x2 + y1 * y2 + z1 * z2;
}

// 計算兩個向量的長度
float vectorLength(float x, float y, float z) {
    return sqrt(x * x + y * y + z * z);
}

// 計算v1 -> v2向量與Z軸的夾角
float calculateAngleFromZAxis(float dx, float dy, float dz) {
    // Z軸的向量 (0, 0, 1)
    float dot = dotProduct(dx, dy, dz, 0.0f, 0.0f, 1.0f);
    float lengthV = vectorLength(dx, dy, dz);
    float lengthZ = 1.0f; // Z軸長度
    return acos(dot / (lengthV * lengthZ)) * (180.0f / M_PI); // 轉換為角度
}

// 依據v1與v2的向量計算旋轉角度
void rotateBasedOnVector() {
    // 方向向量 v1 -> v2
    float dx = v2[0] - v1[0];
    float dy = v2[1] - v1[1];
    float dz = v2[2] - v1[2];

    // 計算旋轉軸的長度並正規化
    float length = sqrt(dx * dx + dy * dy + dz * dz);
    if (length == 0) return;  // 避免除以0
    dx /= length;
    dy /= length;
    dz /= length;

    // 計算旋轉角度的弧度
    float radian = selfRotationAngle * (M_PI / 180.0f);
    float cosA = cos(radian);
    float sinA = sin(radian);
    float oneMinusCos = 1.0f - cosA;

    // 旋轉每個頂點
    for (int i = 0; i < 8; i++) {
        float x = vertices[i][0];
        float y = vertices[i][1];
        float z = vertices[i][2];

        // 使用 Rodrigues' 旋轉公式計算旋轉後的坐標
        float newX = x * (cosA + dx * dx * oneMinusCos) + y * (dx * dy * oneMinusCos - dz * sinA) + z * (dx * dz * oneMinusCos + dy * sinA);
        float newY = x * (dy * dx * oneMinusCos + dz * sinA) + y * (cosA + dy * dy * oneMinusCos) + z * (dy * dz * oneMinusCos - dx * sinA);
        float newZ = x * (dz * dx * oneMinusCos - dy * sinA) + y * (dz * dy * oneMinusCos + dx * sinA) + z * (cosA + dz * dz * oneMinusCos);

        // 更新頂點位置
        vertices[i][0] = newX;
        vertices[i][1] = newY;
        vertices[i][2] = newZ;
    }

    // 重新繪製
    glutPostRedisplay();
}


// 處理鍵盤輸入
void keyboard(unsigned char key, int x, int y) {
    switch (key) {
    case 'w': posY += 0.1f; break;
    case 's': posY -= 0.1f; break;
    case 'a': posX -= 0.1f; break;
    case 'd': posX += 0.1f; break;
    case 'q': posZ += 0.1f; break;
    case 'e': posZ -= 0.1f; break;
    case 'i': angleX += 5.0f; isT = false; break;
    case 'k': angleX -= 5.0f; isT = false; break;
    case 'j': angleY += 5.0f; isT = false; break;
    case 'l': angleY -= 5.0f; isT = false; break;
    case 'u': angleZ += 5.0f; isT = false; break;
    case 'o': angleZ -= 5.0f; isT = false; break;
    case '+': scale *= 1.1f; break;
    case '-': scale /= 1.1f; break;
    case 'r':  // 重置
        posX = posY = posZ = 0.0f;
        angleX = angleY = angleZ = 0.0f;
        scale = 1.0f;
        isT = false;
        break;
    case 't':  // 按t鍵讓方塊旋轉
        rotateAroundVector(&posX, &posY, &posZ, 5.0,0);
        isT = true;
        break;
    case 'g':  // 按t鍵讓方塊旋轉
        rotateAroundVector(&posX, &posY, &posZ, -5.0,0);
        isT = true;
        break;
    case 'y':  // 按t鍵讓方塊旋轉
        rotateAroundVector(&posX, &posY, &posZ, 5.0,1);
        isT = true;
        break;
    case 'h':  // 按t鍵讓方塊旋轉
        rotateAroundVector(&posX, &posY, &posZ, -5.0,1);
        isT = true;
        break;
    case 27: exit(0);
    }
    glutPostRedisplay();
}

// 顯示繪圖
void display() {
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glLoadIdentity();
    
    // 調整視角
    gluLookAt(3, 3, 3, 0, 0, 0, 0, 1, 0);
    drawAxes();
    drawLine();  // 顯示座標 v1 和 v2 之間的連線
    drawCube();  // 顯示方塊
    glBegin(GL_LINES);
    if (hasPoint) {
        // 原始點
        glVertex3f(pointX, pointY, pointZ);
        // 對蹠點
        glVertex3f(-pointX, -pointY, -pointZ);
    }
    glEnd();

    if (_mode == GL_SMOOTH) {
        glBegin(GL_QUADS);
        glColor3f(0, 1, 0); // 紅色 - 前面
        glVertex3fv(vertices[0]);
        glVertex3fv(vertices[1]);
        glVertex3fv(vertices[2]);
        glVertex3fv(vertices[3]);

        glColor3f(1, 0, 0); // 綠色 - 後面
        glVertex3fv(vertices[4]);
        glVertex3fv(vertices[5]);
        glVertex3fv(vertices[6]);
        glVertex3fv(vertices[7]);

        glColor3f(1, 1, 0); // 藍色 - 左側
        glVertex3fv(vertices[4]);
        glVertex3fv(vertices[0]);
        glVertex3fv(vertices[3]);
        glVertex3fv(vertices[7]);

        glColor3f(0, 0, 1); // 黃色 - 右側
        glVertex3fv(vertices[1]);
        glVertex3fv(vertices[5]);
        glVertex3fv(vertices[6]);
        glVertex3fv(vertices[2]);

        glColor3f(0, 1, 1); // 紫色 - 上面
        glVertex3fv(vertices[3]);
        glVertex3fv(vertices[2]);
        glVertex3fv(vertices[6]);
        glVertex3fv(vertices[7]);

        glColor3f(1, 0, 1); // 青色 - 下面
        glVertex3fv(vertices[0]);
        glVertex3fv(vertices[1]);
        glVertex3fv(vertices[5]);
        glVertex3fv(vertices[4]);
        glEnd();
    }
    else if (_mode == GL_FLAT) {
        // **FLAT 模式：使用兩種顏色，但有漸變**
        glBegin(GL_POLYGON);
        glClear(GL_COLOR_BUFFER_BIT);
        glClearColor(0.0f, 0.0f, 1.0f, 1.0f);
        glEnd();
    }

    glutSwapBuffers();
}

// 處理視窗重整
void reshape(int w, int h) {
    glViewport(0, 0, w, h);
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    gluPerspective(45.0, (float)w / (float)h, 1.0, 10.0);
    glMatrixMode(GL_MODELVIEW);
}

// 解析命令列引數
void parseArguments(int argc, char** argv) {
    if (argc >= 4) {
        v1[0] = atof(argv[1]);
        v1[1] = atof(argv[2]);
        v1[2] = atof(argv[3]);
    }
    if (argc >= 7) {
        v2[0] = atof(argv[4]);
        v2[1] = atof(argv[5]);
        v2[2] = atof(argv[6]);
    }
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

    

    glutSwapBuffers();
}

int main(int argc, char** argv) {
    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB | GLUT_DEPTH);
    glutInitWindowSize(800, 600);
    glutCreateWindow("FreeGLUT 3D Cube Rotation");

    parseArguments(argc, argv);  // 解析座標引數

    glEnable(GL_DEPTH_TEST);

    glutDisplayFunc(display);
    //glutDisplayFunc(RenderScene);
    glutMouseFunc(mouseClick);
    glutReshapeFunc(reshape);
    glutKeyboardFunc(keyboard);
    glutCreateMenu(Menu);
    glutAddMenuEntry("Smooth", SMOOTH);
    glutAddMenuEntry("Flat (Two Colors Gradient)", FLAT);
    glutAttachMenu(GLUT_RIGHT_BUTTON);
    
    glutMainLoop();
    return 0;
}
