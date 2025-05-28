#include <stdio.h>
#include <stdlib.h>
#include <GL/freeglut.h>
#include <math.h>    // For fabs, round, fmax, fmin, floor, ceilf
#include <time.h>    // For srand, rand
#include <algorithm> // For std::min, std::max

#define MAX_VERTICES 100
#define MAX_FILLED_CELLS MAX_VERTICES *MAX_POSSIBLE_GRID_SIZE *MAX_POSSIBLE_GRID_SIZE
// --- Configuration ---
int gridSize = 21;
float gridAreaSize = 1.0f;
bool midpointLineRasterizationOn = true;
const int ANIMATION_DELAY_MS = 30;
const int POLYGON_ANIMATION_DELAY_MS = 10;
const float BARY_EPSILON = 1e-4f; // For float comparisons
const float FILL_EPSILON = 1e-4f; // Epsilon specifically for scanline fill calculations if needed
#define MAX_POSSIBLE_GRID_SIZE 30
// ---------------------

typedef struct { float r, g, b; } Color;
typedef struct { int x, y; Color color; } Vertex;
typedef struct { int x, y; Color color; } ColoredCell;
typedef struct { float x; Color color; } IntersectionPoint;

Vertex vertices[MAX_VERTICES];
int vertexCount = 0;
ColoredCell filledCells[MAX_FILLED_CELLS];
int filledCellCount = 0;

bool isLineAnimating = false;
Vertex lineAnimStartVertex, lineAnimEndVertex;
int lineAnim_curX, lineAnim_curY, lineAnim_dx_abs, lineAnim_dy_abs, lineAnim_sx, lineAnim_sy, lineAnim_err, lineAnim_totalActualSteps;

bool isPolygonAnimating = false;
ColoredCell coloredCellsForPolygonAnimation[MAX_POSSIBLE_GRID_SIZE * MAX_POSSIBLE_GRID_SIZE];
int coloredCellsForPolygonAnimationCount = 0;
int polygonFillProgress = 0;
bool needsEdgeRedrawAfterFill = false;

ColoredCell activeMarkerCell;
ColoredCell activeMarkerScanlineStart;
ColoredCell activeMarkerScanlineEnd;
bool drawActiveMarkers = false;
Color redMarkerColor = { 1.0f, 0.0f, 0.0f };

// --- Function Prototypes ---
void animateNextPixel(int value);
void startLineAnimation(Vertex v_start, Vertex v_end);
void collectPolygonCellsForAnimation();
void animatePolygonFill(int value);
void startPolygonFillAnimation(bool viaClickClose);
void resetDrawing();
void redrawAllPolygonEdges();
void rasterizeLineInstantly(Vertex v_start, Vertex v_end);
void addVertex(int x, int y, bool useExistingColor = false, Color existingColor = { 0, 0, 0 });

Color createRandomColor() {
    Color c; c.r = (float)rand() / RAND_MAX; c.g = (float)rand() / RAND_MAX; c.b = (float)rand() / RAND_MAX; return c;
}

void addFilledCell(int x, int y, Color c) {
    for (int i = 0; i < filledCellCount; ++i) {
        if (filledCells[i].x == x && filledCells[i].y == y) {
            filledCells[i].color = c;
            return;
        }
    }
    if (filledCellCount < MAX_FILLED_CELLS) {
        filledCells[filledCellCount].x = x;
        filledCells[filledCellCount].y = y;
        filledCells[filledCellCount].color = c;
        filledCellCount++;
    }
}

void addVertex(int x, int y, bool useExistingColor, Color existingColorIfProvided) {
    if (vertexCount < MAX_VERTICES) {
        vertices[vertexCount].x = x;
        vertices[vertexCount].y = y;
        if (useExistingColor) vertices[vertexCount].color = existingColorIfProvided;
        else vertices[vertexCount].color = createRandomColor();
        addFilledCell(x, y, vertices[vertexCount].color);
        vertexCount++;
        glutPostRedisplay();
        if (vertexCount >= 2 && !isLineAnimating && !isPolygonAnimating) {
            startLineAnimation(vertices[vertexCount - 2], vertices[vertexCount - 1]);
        }
    }
}

Color interpolateColor(Color c1, Color c2, float t) {
    Color c_interp; t = fmax(0.0f, fmin(1.0f, t));
    c_interp.r = (1.0f - t) * c1.r + t * c2.r;
    c_interp.g = (1.0f - t) * c1.g + t * c2.g;
    c_interp.b = (1.0f - t) * c1.b + t * c2.b;
    return c_interp;
}

void rasterizeLineInstantly(Vertex v_start, Vertex v_end) {
    int x0 = v_start.x, y0 = v_start.y, x1 = v_end.x, y1 = v_end.y;
    Color c0 = v_start.color, c1 = v_end.color;
    int dx = abs(x1 - x0), dy = abs(y1 - y0);
    int sx = (x0 < x1) ? 1 : -1, sy = (y0 < y1) ? 1 : -1;
    int err = dx - dy;
    int curX = x0, curY = y0;

    if (x0 == x1 && y0 == y1) { addFilledCell(curX, curY, c0); return; }
    addFilledCell(curX, curY, c0);

    while (curX != x1 || curY != y1) {
        int prevX = curX, prevY = curY;
        int e2 = 2 * err;
        if (e2 > -dy) { err -= dy; curX += sx; }
        if (e2 < dx) { err += dx; curY += sy; }
        if (curX == prevX && curY == prevY && (curX != x1 || curY != y1)) {
            if (dx == 0 && curY != y1) curY += sy;
            else if (dy == 0 && curX != x1) curX += sx;
            else break;
        }
        if (curX == x1 && curY == y1) addFilledCell(curX, curY, c1);
        else {
            float t_interp = 0.0f;
            int total_dist_dx = abs(x1 - x0), total_dist_dy = abs(y1 - y0);
            if (total_dist_dx >= total_dist_dy) {
                if (total_dist_dx == 0) t_interp = 0.0f;
                else t_interp = (float)abs(curX - x0) / total_dist_dx;
            }
            else {
                if (total_dist_dy == 0) t_interp = 0.0f;
                else t_interp = (float)abs(curY - y0) / total_dist_dy;
            }
            addFilledCell(curX, curY, interpolateColor(c0, c1, t_interp));
        }
    }
    addFilledCell(x1, y1, c1); // Ensure endpoint
}

void startLineAnimation(Vertex v_start, Vertex v_end) {
    if (!midpointLineRasterizationOn) { rasterizeLineInstantly(v_start, v_end); glutPostRedisplay(); return; }
    isLineAnimating = true; lineAnimStartVertex = v_start; lineAnimEndVertex = v_end;
    lineAnim_curX = v_start.x; lineAnim_curY = v_start.y;
    lineAnim_dx_abs = abs(v_end.x - v_start.x); lineAnim_dy_abs = abs(v_end.y - v_start.y);
    lineAnim_sx = (v_start.x < v_end.x) ? 1 : -1; lineAnim_sy = (v_start.y < v_end.y) ? 1 : -1;
    lineAnim_err = lineAnim_dx_abs - lineAnim_dy_abs;
    addFilledCell(lineAnim_curX, lineAnim_curY, lineAnimStartVertex.color);
    if (lineAnim_curX == lineAnimEndVertex.x && lineAnim_curY == lineAnimEndVertex.y) {
        isLineAnimating = false; addFilledCell(lineAnim_curX, lineAnim_curY, lineAnimEndVertex.color); glutPostRedisplay();
    }
    else {
        glutPostRedisplay(); glutTimerFunc(ANIMATION_DELAY_MS, animateNextPixel, 0);
    }
}

void animateNextPixel(int value) {
    if (!isLineAnimating) return;
    if (lineAnim_curX == lineAnimEndVertex.x && lineAnim_curY == lineAnimEndVertex.y) {
        isLineAnimating = false; addFilledCell(lineAnim_curX, lineAnim_curY, lineAnimEndVertex.color); glutPostRedisplay(); return;
    }
    int prevX = lineAnim_curX, prevY = lineAnim_curY;
    int e2 = 2 * lineAnim_err;
    if (e2 > -lineAnim_dy_abs) { lineAnim_err -= lineAnim_dy_abs; lineAnim_curX += lineAnim_sx; }
    if (e2 < lineAnim_dx_abs) { lineAnim_err += lineAnim_dx_abs; lineAnim_curY += lineAnim_sy; }
    if (lineAnim_curX == prevX && lineAnim_curY == prevY && (lineAnim_curX != lineAnimEndVertex.x || lineAnim_curY != lineAnimEndVertex.y)) {
        if (lineAnim_dx_abs == 0 && lineAnim_curY != lineAnimEndVertex.y) lineAnim_curY += lineAnim_sy;
        else if (lineAnim_dy_abs == 0 && lineAnim_curX != lineAnimEndVertex.x) lineAnim_curX += lineAnim_sx;
    }

    if (lineAnim_curX == lineAnimEndVertex.x && lineAnim_curY == lineAnimEndVertex.y) {
        addFilledCell(lineAnim_curX, lineAnim_curY, lineAnimEndVertex.color); isLineAnimating = false;
    }
    else {
        float t = 0.0f;
        int total_dx = abs(lineAnimEndVertex.x - lineAnimStartVertex.x);
        int total_dy = abs(lineAnimEndVertex.y - lineAnimStartVertex.y);
        if (total_dx >= total_dy) {
            if (total_dx == 0) t = (lineAnim_curY == lineAnimStartVertex.y) ? 0.0f : 1.0f;
            else t = (float)abs(lineAnim_curX - lineAnimStartVertex.x) / total_dx;
        }
        else {
            if (total_dy == 0) t = (lineAnim_curX == lineAnimStartVertex.x) ? 0.0f : 1.0f;
            else t = (float)abs(lineAnim_curY - lineAnimStartVertex.y) / total_dy;
        }
        addFilledCell(lineAnim_curX, lineAnim_curY, interpolateColor(lineAnimStartVertex.color, lineAnimEndVertex.color, t));
    }
    glutPostRedisplay();
    if (isLineAnimating) glutTimerFunc(ANIMATION_DELAY_MS, animateNextPixel, 0);
}

int compareColoredCellsForSort(const void* a, const void* b) {
    ColoredCell* cA = (ColoredCell*)a; ColoredCell* cB = (ColoredCell*)b;
    if (cA->y < cB->y) return -1; if (cA->y > cB->y) return 1;
    if (cA->x < cB->x) return -1; if (cA->x > cB->x) return 1; return 0;
}
int compareIntersectionPoints(const void* a, const void* b) {
    IntersectionPoint* pA = (IntersectionPoint*)a; IntersectionPoint* pB = (IntersectionPoint*)b;
    if (pA->x < pB->x) return -1; if (pA->x > pB->x) return 1; return 0;
}

void collectPolygonCellsForAnimation() {
    coloredCellsForPolygonAnimationCount = 0;
    polygonFillProgress = 0;
    if (vertexCount < 3) return;

    int minY_poly = gridSize + 1, maxY_poly = -1;
    for (int i = 0; i < vertexCount; ++i) {
        minY_poly = std::min(minY_poly, vertices[i].y);
        maxY_poly = std::max(maxY_poly, vertices[i].y);
    }
    minY_poly = std::max(0, minY_poly);
    maxY_poly = std::min(gridSize - 1, maxY_poly);

    IntersectionPoint scanline_intersections[MAX_VERTICES];

    for (int y_scan_grid = minY_poly; y_scan_grid <= maxY_poly; ++y_scan_grid) {
        float y_scan_line = (float)y_scan_grid; // Scan along grid lines
        int num_intersections = 0;

        for (int i = 0; i < vertexCount; ++i) {
            Vertex p1 = vertices[i];
            Vertex p2 = vertices[(i + 1) % vertexCount];

            float y1_f = (float)p1.y; float y2_f = (float)p2.y;
            float x1_f = (float)p1.x; float x2_f = (float)p2.x;

            if (((y1_f <= y_scan_line) && (y2_f > y_scan_line)) || ((y2_f <= y_scan_line) && (y1_f > y_scan_line))) {
                float intersect_x;
                if (fabs(y2_f - y1_f) < BARY_EPSILON) intersect_x = x1_f;
                else intersect_x = (y_scan_line - y1_f) * (x2_f - x1_f) / (y2_f - y1_f) + x1_f;

                float t_edge;
                if (fabs(y2_f - y1_f) > BARY_EPSILON) t_edge = (y_scan_line - y1_f) / (y2_f - y1_f);
                else if (fabs(x2_f - x1_f) > BARY_EPSILON) t_edge = (intersect_x - x1_f) / (x2_f - x1_f);
                else t_edge = 0.0f;
                t_edge = fmax(0.0f, fmin(1.0f, t_edge));

                if (num_intersections < MAX_VERTICES) {
                    scanline_intersections[num_intersections].x = intersect_x;
                    scanline_intersections[num_intersections].color = interpolateColor(p1.color, p2.color, t_edge);
                    num_intersections++;
                }
            }
        }

        if (num_intersections > 1) {
            qsort(scanline_intersections, num_intersections, sizeof(IntersectionPoint), compareIntersectionPoints);
            for (int k = 0; k < num_intersections - 1; k += 2) {
                IntersectionPoint int_p1 = scanline_intersections[k];
                IntersectionPoint int_p2 = scanline_intersections[k + 1];

                // Standard scanline: fill from ceil(left_x) to floor(right_x)
                // This often means pixel 'x' is filled if its span [x, x+1) intersects the fill span.
                // For strict "center in polygon":
                // int start_gx = (int)ceilf(int_p1.x - 0.5f);
                // int end_gx   = (int)floorf(int_p2.x - 0.5f - FILL_EPSILON); // Subtract small epsilon before floor

                // Using common integer pixel fill rule:
                // Inclusive start, exclusive end for continuous coordinates, maps to:
                // Inclusive start pixel, inclusive end pixel if endpoint is part of pixel.
                int start_gx = (int)ceilf(int_p1.x);
                int end_gx = (int)floorf(int_p2.x - FILL_EPSILON); // if int_p2.x = 5.0, floor(4.999..)=4. Fills up to pixel 4.
                // This makes the right boundary exclusive.
                // If polygon edge ends exactly at 5.0, pixel 4 is last.

// If we want int_p2.x to be inclusive (pixel at floor(int_p2.x) is filled):
// int end_gx = (int)floorf(int_p2.x);
// Example: [2.0, 5.0] -> ceil(2.0)=2, floor(5.0)=5. Fill 2,3,4,5.
// Example: [2.3, 4.8] -> ceil(2.3)=3, floor(4.8)=4. Fill 3,4. (Pixel 2 might be missed)

// Let's try: round the start, and round the end.
// This fills pixels whose centers are closest to the span.
// start_gx = (int)roundf(int_p1.x);
// end_gx   = (int)roundf(int_p2.x) -1; // if round(end) is exclusive boundary
// This is tricky. The original `ceilf` and `floorf - EPSILON` is a specific convention.

// Sticking to a common interpretation for grid cells:
// Fill pixel gx if any part of [gx, gx+1) is in [int_p1.x, int_p2.x]
// Simplest:
                start_gx = (int)ceilf(int_p1.x);
                end_gx = (int)floorf(int_p2.x);
                // Check if int_p2.x is almost an integer and floor made it too small
                if (fabs(int_p2.x - roundf(int_p2.x)) < FILL_EPSILON && roundf(int_p2.x) > end_gx) {
                    // If int_p2.x is like 4.999999, floor is 4. round is 5.
                    // If int_p2.x is 5.000001, floor is 5. round is 5.
                    // This case is for when int_p2.x is slightly less than an int, e.g. 4.999999... (epsilon effects)
                    // No, this is not quite right.
                }
                // If right boundary int_p2.x is exactly on pixel boundary, e.g. 5.0,
                // floorf(5.0) = 5. So loop will include gx=5.
                // This means pixel 5 (span [5,6)) is included.

                // The most problematic is often the exclusive right boundary.
                // If int_p2.x is 5.0, and it's an *exclusive* boundary for the fill span, then pixel 4 is the last.
                // floorf(int_p2.x - FILL_EPSILON) achieves this.
                end_gx = (int)floorf(int_p2.x - FILL_EPSILON);


                start_gx = std::max(0, std::min(gridSize - 1, start_gx));
                end_gx = std::max(0, std::min(gridSize - 1, end_gx));
                if (start_gx > end_gx) continue;

                for (int gx = start_gx; gx <= end_gx; ++gx) {
                    if (coloredCellsForPolygonAnimationCount >= MAX_POSSIBLE_GRID_SIZE * MAX_POSSIBLE_GRID_SIZE) break;
                    float t_scan;
                    if (fabs(int_p2.x - int_p1.x) > BARY_EPSILON)
                        t_scan = ((float)gx - int_p1.x) / (int_p2.x - int_p1.x); // gx is left edge of pixel
                    else t_scan = 0.0f;
                    t_scan = fmax(0.0f, fmin(1.0f, t_scan));

                    coloredCellsForPolygonAnimation[coloredCellsForPolygonAnimationCount].x = gx;
                    coloredCellsForPolygonAnimation[coloredCellsForPolygonAnimationCount].y = y_scan_grid;
                    coloredCellsForPolygonAnimation[coloredCellsForPolygonAnimationCount].color = interpolateColor(int_p1.color, int_p2.color, t_scan);
                    coloredCellsForPolygonAnimationCount++;
                }
            }
        }
        if (coloredCellsForPolygonAnimationCount >= MAX_POSSIBLE_GRID_SIZE * MAX_POSSIBLE_GRID_SIZE) break;
    }
    if (coloredCellsForPolygonAnimationCount > 0)
        qsort(coloredCellsForPolygonAnimation, coloredCellsForPolygonAnimationCount, sizeof(ColoredCell), compareColoredCellsForSort);
}


void redrawAllPolygonEdges() {
    if (vertexCount < 2) return;
    for (int i = 0; i < vertexCount; ++i) {
        Vertex p1 = vertices[i];
        Vertex p2 = vertices[(i + 1) % vertexCount];
        if (i == vertexCount - 1 && p1.x == vertices[0].x && p1.y == vertices[0].y) {
            // If last vertex is a duplicate of first (closed polygon), this edge is from V0_copy to V0 (zero length)
            continue;
        }
        rasterizeLineInstantly(p1, p2);
    }
    glutPostRedisplay();
}

void animatePolygonFill(int value) {
    if (!isPolygonAnimating) { drawActiveMarkers = false; return; }
    if (polygonFillProgress >= coloredCellsForPolygonAnimationCount) {
        isPolygonAnimating = false; drawActiveMarkers = false;
        if (needsEdgeRedrawAfterFill) { redrawAllPolygonEdges(); needsEdgeRedrawAfterFill = false; }
        glutPostRedisplay(); return;
    }
    ColoredCell cellToDraw = coloredCellsForPolygonAnimation[polygonFillProgress];
    drawActiveMarkers = true; activeMarkerCell = cellToDraw;
    int currentY = cellToDraw.y; activeMarkerScanlineStart = cellToDraw;
    for (int i = polygonFillProgress - 1; i >= 0; --i) {
        if (coloredCellsForPolygonAnimation[i].y == currentY && coloredCellsForPolygonAnimation[i].x == activeMarkerScanlineStart.x - 1)
            activeMarkerScanlineStart = coloredCellsForPolygonAnimation[i];
        else if (coloredCellsForPolygonAnimation[i].y != currentY || coloredCellsForPolygonAnimation[i].x < activeMarkerScanlineStart.x - 1) break;
    }
    activeMarkerScanlineEnd = cellToDraw;
    for (int i = polygonFillProgress + 1; i < coloredCellsForPolygonAnimationCount; ++i) {
        if (coloredCellsForPolygonAnimation[i].y == currentY && coloredCellsForPolygonAnimation[i].x == activeMarkerScanlineEnd.x + 1)
            activeMarkerScanlineEnd = coloredCellsForPolygonAnimation[i];
        else if (coloredCellsForPolygonAnimation[i].y != currentY || coloredCellsForPolygonAnimation[i].x > activeMarkerScanlineEnd.x + 1) break;
    }
    addFilledCell(cellToDraw.x, cellToDraw.y, cellToDraw.color);
    polygonFillProgress++; glutPostRedisplay();
    if (polygonFillProgress < coloredCellsForPolygonAnimationCount)
        glutTimerFunc(POLYGON_ANIMATION_DELAY_MS, animatePolygonFill, 0);
    else {
        isPolygonAnimating = false; drawActiveMarkers = false;
        if (needsEdgeRedrawAfterFill) { redrawAllPolygonEdges(); needsEdgeRedrawAfterFill = false; }
        glutPostRedisplay();
    }
}

void startPolygonFillAnimation(bool viaClickClose) {
    if (viaClickClose && vertexCount < 4) { printf("Need at least 3 unique vertices (4 total after closing by click) to fill.\n"); return; }
    if (!viaClickClose && vertexCount < 3) { printf("Need at least 3 vertices to fill.\n"); return; }
    isPolygonAnimating = true; drawActiveMarkers = false; needsEdgeRedrawAfterFill = true;
    collectPolygonCellsForAnimation(); polygonFillProgress = 0;
    if (midpointLineRasterizationOn) {
        if (coloredCellsForPolygonAnimationCount > 0) glutTimerFunc(POLYGON_ANIMATION_DELAY_MS, animatePolygonFill, 0);
        else {
            isPolygonAnimating = false; drawActiveMarkers = false;
            if (needsEdgeRedrawAfterFill) { redrawAllPolygonEdges(); needsEdgeRedrawAfterFill = false; }
            glutPostRedisplay();
        }
    }
    else {
        for (int i = 0; i < coloredCellsForPolygonAnimationCount; ++i)
            addFilledCell(coloredCellsForPolygonAnimation[i].x, coloredCellsForPolygonAnimation[i].y, coloredCellsForPolygonAnimation[i].color);
        isPolygonAnimating = false; drawActiveMarkers = false;
        coloredCellsForPolygonAnimationCount = 0; polygonFillProgress = 0;
        if (needsEdgeRedrawAfterFill) { redrawAllPolygonEdges(); needsEdgeRedrawAfterFill = false; }
        glutPostRedisplay();
    }
}

void drawGrid() {
    float halfW = gridAreaSize / 2.0f, step = gridAreaSize / gridSize;
    for (int i = 0; i < filledCellCount; ++i) {
        float worldX = filledCells[i].x * step - halfW, worldY = filledCells[i].y * step - halfW;
        glColor3f(filledCells[i].color.r, filledCells[i].color.g, filledCells[i].color.b);
        glRectf(worldX, worldY, worldX + step, worldY + step);
    }
    if (isLineAnimating && midpointLineRasterizationOn) {
        glColor3f(0.0f, 1.0f, 0.0f);
        float worldX = lineAnim_curX * step - halfW, worldY = lineAnim_curY * step - halfW;
        glRectf(worldX, worldY, worldX + step, worldY + step);
    }
    if (isPolygonAnimating && drawActiveMarkers && polygonFillProgress < coloredCellsForPolygonAnimationCount) {
        glColor3f(redMarkerColor.r, redMarkerColor.g, redMarkerColor.b);
        float worldMarkerX = activeMarkerCell.x * step - halfW, worldMarkerY = activeMarkerCell.y * step - halfW;
        glRectf(worldMarkerX, worldMarkerY, worldMarkerX + step, worldMarkerY + step);
        worldMarkerX = activeMarkerScanlineStart.x * step - halfW; worldMarkerY = activeMarkerScanlineStart.y * step - halfW;
        glRectf(worldMarkerX, worldMarkerY, worldMarkerX + step, worldMarkerY + step);
        if (activeMarkerScanlineStart.x != activeMarkerScanlineEnd.x || activeMarkerScanlineStart.y != activeMarkerScanlineEnd.y) {
            worldMarkerX = activeMarkerScanlineEnd.x * step - halfW; worldMarkerY = activeMarkerScanlineEnd.y * step - halfW;
            glRectf(worldMarkerX, worldMarkerY, worldMarkerX + step, worldMarkerY + step);
        }
    }
    glColor3f(0.25f, 0.25f, 0.25f); glBegin(GL_LINES);
    for (int i = 0; i <= gridSize; ++i) {
        float pos = -halfW + i * step;
        glVertex2f(pos, -halfW); glVertex2f(pos, halfW);
        glVertex2f(-halfW, pos); glVertex2f(halfW, pos);
    }
    glEnd();
}

void display() { glClear(GL_COLOR_BUFFER_BIT); glLoadIdentity(); drawGrid(); glutSwapBuffers(); }
void reshape(int w, int h) {
    glViewport(0, 0, w, h); glMatrixMode(GL_PROJECTION); glLoadIdentity();
    float halfE = gridAreaSize / 2.0f, aspect = (float)w / (h == 0 ? 1 : h), padF = 1.1f; halfE *= padF;
    GLdouble l, r, b, t;
    if (aspect >= 1.0) { l = -halfE * aspect; r = halfE * aspect; b = -halfE; t = halfE; }
    else { l = -halfE; r = halfE; b = -halfE / aspect; t = halfE / aspect; }
    gluOrtho2D(l, r, b, t); glMatrixMode(GL_MODELVIEW);
}

void mouse(int button, int state, int x_screen, int y_screen) {
    if (button == GLUT_LEFT_BUTTON && state == GLUT_DOWN) {
        if (isPolygonAnimating || (isLineAnimating && midpointLineRasterizationOn)) return;
        GLdouble modelMatrix[16], projMatrix[16]; GLint viewport[4];
        glGetDoublev(GL_MODELVIEW_MATRIX, modelMatrix); glGetDoublev(GL_PROJECTION_MATRIX, projMatrix); glGetIntegerv(GL_VIEWPORT, viewport);
        float winX = (float)x_screen, winY = (float)viewport[3] - (float)y_screen;
        GLdouble worldX_d, worldY_d, worldZ_d;
        gluUnProject(winX, winY, 0.0, modelMatrix, projMatrix, viewport, &worldX_d, &worldY_d, &worldZ_d);
        float worldX = (float)worldX_d, worldY = (float)worldY_d, halfGridArea = gridAreaSize / 2.0f;

        if (worldX >= -halfGridArea && worldX <= halfGridArea && worldY >= -halfGridArea && worldY <= halfGridArea) {
            float step = gridAreaSize / gridSize;
            int cellX = floor((worldX + halfGridArea) / step);
            int cellY = floor((worldY + halfGridArea) / step);
            cellX = std::max(0, std::min(gridSize - 1, cellX)); cellY = std::max(0, std::min(gridSize - 1, cellY));
            int existingVertexIndex = -1;
            for (int i = 0; i < vertexCount; ++i) if (vertices[i].x == cellX && vertices[i].y == cellY) { existingVertexIndex = i; break; }

            if (vertexCount >= 3 && existingVertexIndex == 0) { // Close polygon
                if (vertexCount < MAX_VERTICES) { addVertex(cellX, cellY, true, vertices[0].color); startPolygonFillAnimation(true); } return;
            }
            if (existingVertexIndex != -1) { // Click existing vertex
                if (vertexCount > 0 && vertexCount < MAX_VERTICES) addVertex(cellX, cellY, true, vertices[existingVertexIndex].color); return;
            }
            if (vertexCount < MAX_VERTICES) addVertex(cellX, cellY, false); // New vertex
        }
    }
}

void resetDrawing() {
    vertexCount = 0; filledCellCount = 0; isLineAnimating = false; isPolygonAnimating = false;
    coloredCellsForPolygonAnimationCount = 0; polygonFillProgress = 0;
    needsEdgeRedrawAfterFill = false; drawActiveMarkers = false;
    glutPostRedisplay();
}

void menu(int value) {
    switch (value) {
    case 1: midpointLineRasterizationOn = !midpointLineRasterizationOn;
        if (isLineAnimating && !midpointLineRasterizationOn) {
            isLineAnimating = false; rasterizeLineInstantly(lineAnimStartVertex, lineAnimEndVertex); glutPostRedisplay();
        } break;
    case 10: gridSize = 10; resetDrawing(); break;
    case 21: gridSize = 21; resetDrawing(); break;
    case 30: gridSize = 30; resetDrawing(); break;
    case 99: resetDrawing(); break;
    }
}

void keyboard(unsigned char key, int x, int y) {
    if (key == 'r' || key == 'R') resetDrawing();
    if (key == ' ') {
        if (vertexCount >= 2 && (!isLineAnimating || !midpointLineRasterizationOn) && !isPolygonAnimating) {
            if (vertexCount < MAX_VERTICES) addVertex(vertices[0].x, vertices[0].y, true, vertices[0].color);
        }
    }
    if (key == 'f' || key == 'F') {
        if ((!isLineAnimating || !midpointLineRasterizationOn) && !isPolygonAnimating) startPolygonFillAnimation(false);
        else printf("Cannot start fill: another animation is in progress or line animation is on.\n");
    }
}

int main(int argc, char** argv) {
    srand((unsigned int)time(NULL));
    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB);
    glutInitWindowSize(700, 700);
    glutCreateWindow("Polygon Scanline Fill - Corrected Boundary Fill");
    glutDisplayFunc(display); glutReshapeFunc(reshape); glutMouseFunc(mouse); glutKeyboardFunc(keyboard);
    glutCreateMenu(menu);
    glutAddMenuEntry("Toggle Line Anim (On/Off)", 1);
    glutAddMenuEntry("--- Grid Size ---", -1);
    glutAddMenuEntry("10x10", 10); glutAddMenuEntry("21x21", 21); glutAddMenuEntry("30x30", 30);
    glutAddMenuEntry("--- Actions ---", -1);
    glutAddMenuEntry("Reset (R)", 99);
    glutAddMenuEntry("Fill Polygon (F / Click 1st Vtx)", -2);
    glutAddMenuEntry("Close Polygon Line (Space)", -3);
    glutAttachMenu(GLUT_RIGHT_BUTTON);
    glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
    printf("LClick: Add Vtx. RClick: Menu. R:Reset. F/Click 1st Vtx:Fill. Space:Close Line.\n");
    printf("Scanline fill uses: start_gx = ceil(left_x), end_gx = floor(right_x - EPSILON)\n");
    glutMainLoop();
    return 0;
}