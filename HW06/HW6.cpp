#include <stdio.h>
#include <stdlib.h>
#include <GL/freeglut.h>

#define MAX_CELLS 1000

int gridSize = 21;
float gridAreaSize = 1.0f;  // 總區域大小為 [-0.5, 0.5]，中心為 (0, 0)

typedef struct {
    int x, y;
} Cell;

Cell clickedCells[MAX_CELLS];
int clickedCount = 0;

int isCellClicked(int x, int y) {
    for (int i = 0; i < clickedCount; ++i) {
        if (clickedCells[i].x == x && clickedCells[i].y == y)
            return 1;
    }
    return 0;
}

void addClickedCell(int x, int y) {
    if (!isCellClicked(x, y) && clickedCount < MAX_CELLS) {
        clickedCells[clickedCount].x = x;
        clickedCells[clickedCount].y = y;
        clickedCount++;
    }
}

void drawGrid() {
    float half = gridAreaSize / 2.0f;
    float step = gridAreaSize / gridSize;

    // 畫已點擊的格子
    for (int i = 0; i < clickedCount; ++i) {
        int x = clickedCells[i].x;
        int y = clickedCells[i].y;

        float left = x * step - half;
        float right = (x + 1) * step - half;
        float bottom = y * step - half;
        float top = (y + 1) * step - half;
        
        glColor3f(1.0f, 0.0f, 0.0f);  // 紅色
        glBegin(GL_QUADS);
        glVertex2f(left, bottom);
        glVertex2f(right, bottom);
        glVertex2f(right, top);
        glVertex2f(left, top);
        glEnd();
    }

    // 畫格線
    glColor3f(1.0, 1.0, 1.0);
    glBegin(GL_LINES);
    for (int i = 0; i <= gridSize ; ++i) {
        float pos = -half + i * step;
        glVertex2f(pos, -half);
        glVertex2f(pos, half);
        glVertex2f(-half, pos);
        glVertex2f(half, pos);
    }
    glEnd();
}

void display() {
    glClear(GL_COLOR_BUFFER_BIT);
    glLoadIdentity();
    drawGrid();
    glutSwapBuffers();
}

void reshape(int w, int h) {
    glViewport(0, 0, w, h);
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    gluOrtho2D(-1.0, 1.0, -1.0, 1.0);  // 固定可視區
    glMatrixMode(GL_MODELVIEW);
}

void mouse(int button, int state, int x, int y) {
    if (button == GLUT_LEFT_BUTTON && state == GLUT_DOWN) {
        int winW = glutGet(GLUT_WINDOW_WIDTH);
        int winH = glutGet(GLUT_WINDOW_HEIGHT);

        // 把像素座標轉成 OpenGL 座標（-1 到 1）
        float fx = (float)x / winW * 2.0f - 1.0f;
        float fy = 1.0f - (float)y / winH * 2.0f;



        // 只考慮落在格線區域內的點
        float half = gridAreaSize / 2.0f;
        if (fx >= -half && fx <= half && fy >= -half && fy <= half) {
            float step = gridAreaSize / gridSize;

            int cellX = (int)((fx + half) / step);
            int cellY = (int)((fy + half) / step);

            int centeredX = cellX - gridSize / 2;
            int centeredY = cellY - gridSize / 2;

            printf("Clicked cell: (%d, %d)\n", centeredX, centeredY);

            addClickedCell(cellX, cellY);
            glutPostRedisplay();
        }
    }
}

void menu(int value) {
    switch (value) {
    case 10:
    case 15:
    case 20:
        gridSize = value * 2 + 1;
        clickedCount = 0;  // 清除點擊紀錄
        glutPostRedisplay();
        break;
    }
}

int main(int argc, char** argv) {
    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB);
    glutInitWindowSize(600, 600);
    glutCreateWindow("中心為 (0,0) 的格線點擊互動");

    glutDisplayFunc(display);
    glutReshapeFunc(reshape);
    glutMouseFunc(mouse);

    glutCreateMenu(menu);
    glutAddMenuEntry("10 x 10", 10);
    glutAddMenuEntry("15 x 15", 15);
    glutAddMenuEntry("20 x 20", 20);
    glutAttachMenu(GLUT_RIGHT_BUTTON);

    glClearColor(0.0, 0.0, 0.0, 1.0);
    glutMainLoop();
    return 0;
}
