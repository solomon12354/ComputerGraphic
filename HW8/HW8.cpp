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

Cell lineCells[MAX_CELLS * 10]; // 比點更多，因為中間會很多格
int lineCellCount = 0;

Cell dots[2];

typedef struct {
    int x, y;
    int color; // 1 = 紅, 2 = 綠, 3 = 藍, 4 = 黃
} ColoredCell;

ColoredCell filledCells[MAX_CELLS * 10];
int filledCount = 0;

int isFilled(int x, int y) {
    for (int i = 0; i < filledCount; ++i)
        if (filledCells[i].x == x && filledCells[i].y == y)
            return 1;
    return 0;
}

void addColoredCell(int x, int y, int color) {
    if (!isFilled(x, y) && filledCount < MAX_CELLS * 10) {
        filledCells[filledCount].x = x;
        filledCells[filledCount].y = y;
        filledCells[filledCount].color = color;
        filledCount++;
    }
}

int sign(int x1, int y1, int x2, int y2, int x3, int y3) {
    return (x1 - x3) * (y2 - y3) - (x2 - x3) * (y1 - y3);
}

int isPointInTriangle(int px, int py, Cell a, Cell b, Cell c) {
    int d1 = sign(px, py, a.x, a.y, b.x, b.y);
    int d2 = sign(px, py, b.x, b.y, c.x, c.y);
    int d3 = sign(px, py, c.x, c.y, a.x, a.y);

    int has_neg = (d1 < 0) || (d2 < 0) || (d3 < 0);
    int has_pos = (d1 > 0) || (d2 > 0) || (d3 > 0);

    return !(has_neg && has_pos);
}


void addMidpointLineCellsColored(int x0, int y0, int x1, int y1) {
    int dx = abs(x1 - x0), dy = abs(y1 - y0);
    int sx = (x0 < x1) ? 1 : -1;
    int sy = (y0 < y1) ? 1 : -1;
    int err = dx - dy;

    while (1) {
        // 不塗終點（保留紅色），也不重塗已經上過色的格子
        if (!(x0 == x1 && y0 == y1) && !isFilled(x0, y0)) {
            addColoredCell(x0, y0, 2); // 初始為綠色
        }

        if (x0 == x1 && y0 == y1) break;

        int e2 = 2 * err;
        int oldX = x0;
        int oldY = y0;

        if (e2 > -dy) {
            err -= dy;
            x0 += sx;
        }
        if (e2 < dx) {
            err += dx;
            y0 += sy;
        }

        // 判斷移動方向並上對應顏色
        if (!(x0 == x1 && y0 == y1) && !isFilled(x0, y0)) {
            int dxStep = x0 - oldX;
            int dyStep = y0 - oldY;

            int color = 2; // 預設綠色

            if (dxStep > 0 && dyStep > 0) {
                color = 3;  //printf("1"); 
            }      // 右上 → 藍色
            else if (dxStep > 0 && dyStep < 0) {
                color = 5; //printf("8");
            } // 右下 → 紫色
            else if (dxStep < 0 && dyStep < 0) {
                color = 6; //printf("6");
            } // 左下 → 白色
            else if (dxStep < 0 && dyStep > 0) {
                color = 4; //printf("3");
            }// 左上 → 黃色
            else color = 2;                                // 其他 → 綠色

            addColoredCell(x0, y0, color);
        }
    }
}


/*
void addMidpointLineCellsColored(int x0, int y0, int x1, int y1) {
    int dx = abs(x1 - x0), dy = abs(y1 - y0);
    int sx = (x0 < x1) ? 1 : -1;
    int sy = (y0 < y1) ? 1 : -1;
    int err = dx - dy;

    // 決定顏色
    int color = 1; // 預設綠
    if (x0 < x1 && y0 < y1) {
        float slope = (float)dy / dx;
        if (slope > 1.0f) color = 3; // 藍
        else if(slope < 1.0f && slope >= 0.0f) color = 2;
        else color = 4; // 黃
    }
    else if (x0 > x1 && y0 < y1) {
        color = 5;
    }
    else if (x0 > x1 && y0 > y1) {
        color = 6;
    }
    else {
        color = 7;
    }

    while (1) {
        // 排除兩端點（保持紅色）
        if (!((x0 == x1 && y0 == y1))) {
            addColoredCell(x0, y0, color);
        }

        if (x0 == x1 && y0 == y1) break;
        int e2 = 2 * err;
        if (e2 > -dy) { err -= dy; x0 += sx; }
        if (e2 < dx) { err += dx; y0 += sy; }
    }
}
*/



int isLineCell(int x, int y) {
    for (int i = 0; i < lineCellCount; ++i)
        if (lineCells[i].x == x && lineCells[i].y == y)
            return 1;
    return 0;
}

void addLineCell(int x, int y) {
    if (!isLineCell(x, y) && lineCellCount < MAX_CELLS * 10) {
        lineCells[lineCellCount].x = x;
        lineCells[lineCellCount].y = y;
        lineCellCount++;
    }
}


int isCellClicked(int x, int y) {
    for (int i = 0; i < clickedCount; ++i) {
        if (clickedCells[i].x == x && clickedCells[i].y == y)
            return 1;
    }
    return 0;
}

void addClickedCell(int x, int y) {
    if (!isFilled(x, y) && clickedCount < MAX_CELLS) {
        clickedCells[clickedCount].x = x;
        clickedCells[clickedCount].y = y;
        clickedCount++;

        addColoredCell(x, y, 1); // 紅色
    }
}


void addLineCells(int x0, int y0, int x1, int y1) {
    int dx = abs(x1 - x0), dy = abs(y1 - y0);
    int sx = (x0 < x1) ? 1 : -1;
    int sy = (y0 < y1) ? 1 : -1;
    int err = dx - dy;

    while (1) {
        addLineCell(x0, y0);
        if (x0 == x1 && y0 == y1) break;
        int e2 = 2 * err;
        if (e2 > -dy) { err -= dy; x0 += sx; }
        if (e2 < dx) { err += dx; y0 += sy; }
    }
}


void addMidpointLineCells(int x0, int y0, int x1, int y1) {
    int dx = abs(x1 - x0), dy = abs(y1 - y0);
    int sx = (x0 < x1) ? 1 : -1;
    int sy = (y0 < y1) ? 1 : -1;
    int err = dx - dy;

    while (1) {
        // 排除兩端點
        if (!((x0 == x1 && y0 == y1) || (x0 == clickedCells[clickedCount - 2].x && y0 == clickedCells[clickedCount - 2].y))) {
            addLineCell(x0, y0);
        }

        if (x0 == x1 && y0 == y1) break;
        int e2 = 2 * err;
        if (e2 > -dy) { err -= dy; x0 += sx; }
        if (e2 < dx) { err += dx; y0 += sy; }
    }
}



void drawGrid() {
    float half = gridAreaSize / 2.0f;
    float step = gridAreaSize / gridSize;

    // 畫紅色：被點擊的格子
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

    for (int i = 0; i < filledCount; ++i) {
        int x = filledCells[i].x;
        int y = filledCells[i].y;

        float left = x * step - half;
        float right = (x + 1) * step - half;
        float bottom = y * step - half;
        float top = (y + 1) * step - half;

        switch (filledCells[i].color) {
        case 1: glColor3f(1.0f, 0.0f, 0.0f); break; // 紅
        case 2: glColor3f(0.0f, 1.0f, 0.0f); break; // 綠
        case 3: glColor3f(0.0f, 0.0f, 1.0f); break; // 藍
        case 4: glColor3f(1.0f, 1.0f, 0.0f); break; // 黃
        case 5: glColor3f(1.0f, 0.0f, 1.0f); break; // 紅
        case 6: glColor3f(1.0f, 1.0f, 1.0f); break; // 紅
        case 7: glColor3f(0.0f, 1.0f, 1.0f); break; // 紅
        }

        glBegin(GL_QUADS);
        glVertex2f(left, bottom);
        glVertex2f(right, bottom);
        glVertex2f(right, top);
        glVertex2f(left, top);
        glEnd();
    }




    // 每兩個點畫一條線
    glColor3f(0.0f, 1.0f, 0.0f); // 綠色線條
    glBegin(GL_LINES);
    for (int i = 0; i + 1 < clickedCount; i += 2) {
        float x1 = (clickedCells[i].x + 0.5f) * step - half;
        float y1 = (clickedCells[i].y + 0.5f) * step - half;
        float x2 = (clickedCells[i + 1].x + 0.5f) * step - half;
        float y2 = (clickedCells[i + 1].y + 0.5f) * step - half;

        glVertex2f(x1, y1);
        glVertex2f(x2, y2);
    }
    glEnd();

    // 畫格線
    glColor3f(1.0, 1.0, 1.0);
    glBegin(GL_LINES);
    for (int i = 0; i <= gridSize; ++i) {
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
            addClickedCell(cellX, cellY);

            if (clickedCount >= 2) {
                int x0 = clickedCells[clickedCount - 2].x;
                int y0 = clickedCells[clickedCount - 2].y;
                int x1 = clickedCells[clickedCount - 1].x;
                int y1 = clickedCells[clickedCount - 1].y;
                dots[0].x = x0;
                dots[0].y = y0;
                dots[1].x = x1;
                dots[1].y = y1;
                printf("x0 = %d, y0 = %d, x1 = %d, y1 = %d\n", x0, y0, x1, y1);
                if (x1 - x0 >= 0 && y1 - y0 >= 0) {
                    if ((y1 - y0) / (x1 - x0) < 1.0f) {
                        printf("Region: 1\n");
                    }
                    else {
                        printf("Region: 2\n");
                    }
                }
                else if (x1 - x0 < 0 && y1 - y0 >= 0) {
                    if ((y1 - y0) / (x1 - x0) > -1.0f) {
                        printf("Region: 4\n");
                    }
                    else {
                        printf("Region: 3\n");
                    }
                }
                else if (x1 - x0 < 0 && y1 - y0 < 0) {
                    if ((y1 - y0) / (x1 - x0) < 1.0f) {
                        printf("Region: 5\n");
                    }
                    else {
                        printf("Region: 6\n");
                    }
                }
                else {
                    if ((y1 - y0) / (x1 - x0) > -1.0f) {
                        printf("Region: 8\n");
                    }
                    else {
                        printf("Region: 7\n");
                    }
                }
                addMidpointLineCellsColored(x0, y0, x1, y1);
            }
            if (clickedCount >= 3) {
                int x0 = clickedCells[clickedCount - 3].x;
                int y0 = clickedCells[clickedCount - 3].y;
                int x1 = clickedCells[clickedCount - 1].x;
                int y1 = clickedCells[clickedCount - 1].y;
                dots[0].x = x0;
                dots[0].y = y0;
                dots[1].x = x1;
                dots[1].y = y1;
                printf("x0 = %d, y0 = %d, x1 = %d, y1 = %d\n", x0, y0, x1, y1);
                if (x1 - x0 >= 0 && y1 - y0 >= 0) {
                    if ((y1 - y0) / (x1 - x0) < 1.0f) {
                        printf("Region: 1\n");
                    }
                    else {
                        printf("Region: 2\n");
                    }
                }
                else if (x1 - x0 < 0 && y1 - y0 >= 0) {
                    if ((y1 - y0) / (x1 - x0) > -1.0f) {
                        printf("Region: 4\n");
                    }
                    else {
                        printf("Region: 3\n");
                    }
                }
                else if (x1 - x0 < 0 && y1 - y0 < 0) {
                    if ((y1 - y0) / (x1 - x0) < 1.0f) {
                        printf("Region: 5\n");
                    }
                    else {
                        printf("Region: 6\n");
                    }
                }
                else {
                    if ((y1 - y0) / (x1 - x0) > -1.0f) {
                        printf("Region: 8\n");
                    }
                    else {
                        printf("Region: 7\n");
                    }
                }
                addMidpointLineCellsColored(x0, y0, x1, y1);

                Cell a = clickedCells[clickedCount - 3];
                Cell b = clickedCells[clickedCount - 2];
                Cell c = clickedCells[clickedCount - 1];

                for (int x = 0; x < gridSize; ++x) {
                    for (int y = 0; y < gridSize; ++y) {
                        if (isPointInTriangle(x, y, a, b, c)) {
                            addColoredCell(x, y, 3); // 3 = 藍色
                        }
                    }
                }
                
                glutPostRedisplay();
            }


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
    case 21:
        filledCount = 0; // 清空顏色陣列
        clickedCount = 0;
        lineCellCount = 0;
        glutPostRedisplay(); // 重新繪圖
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
    glutAddMenuEntry("Reset", 21);
    glutAttachMenu(GLUT_RIGHT_BUTTON);

    glClearColor(0.0, 0.0, 0.0, 1.0);
    glutMainLoop();
    return 0;
}
