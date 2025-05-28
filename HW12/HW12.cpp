#include <GL/freeglut.h>
#include <cmath>

#define X .525731112119133606
#define Z .850650808352039932

static GLfloat vdata[12][3] = {
    {-X, 0.0, Z}, {X, 0.0, Z}, {-X, 0.0, -Z}, {X, 0.0, -Z},
    {0.0, Z, X}, {0.0, Z, -X}, {0.0, -Z, X}, {0.0, -Z, -X},
    {Z, X, 0.0}, {-Z, X, 0.0}, {Z, -X, 0.0}, {-Z, -X, 0.0}
};

static GLuint tindices[20][3] = {
    {1,4,0}, {4,9,0}, {4,5,9}, {8,5,4}, {1,8,4},
    {1,10,8}, {10,3,8}, {8,3,5}, {3,2,5}, {3,7,2},
    {3,10,7}, {10,6,7}, {6,11,7}, {6,0,11}, {6,1,0},
    {10,1,6}, {11,0,9}, {2,11,9}, {5,2,9}, {11,2,7}
};

int rotateX = 0, rotateY = 0;
int polygonMode = GL_FILL;
int subdivDepth = 0;

void normalize(GLfloat v[3]) {
    GLfloat d = sqrt(v[0] * v[0] + v[1] * v[1] + v[2] * v[2]);
    if (d == 0.0) return;
    v[0] /= d; v[1] /= d; v[2] /= d;
}

void drawTriangle(GLfloat* v1, GLfloat* v2, GLfloat* v3) {
    glNormal3fv(v1); glVertex3fv(v1);
    glNormal3fv(v2); glVertex3fv(v2);
    glNormal3fv(v3); glVertex3fv(v3);
}

void subdivide(GLfloat* v1, GLfloat* v2, GLfloat* v3, int depth) {
    if (depth == 0) {
        drawTriangle(v1, v2, v3);
        return;
    }

    GLfloat v12[3], v23[3], v31[3];
    for (int i = 0; i < 3; i++) {
        v12[i] = (v1[i] + v2[i]) / 2.0;
        v23[i] = (v2[i] + v3[i]) / 2.0;
        v31[i] = (v3[i] + v1[i]) / 2.0;
    }
    normalize(v12); normalize(v23); normalize(v31);

    subdivide(v1, v12, v31, depth - 1);
    subdivide(v2, v23, v12, depth - 1);
    subdivide(v3, v31, v23, depth - 1);
    subdivide(v12, v23, v31, depth - 1);
}

void drawIcosahedron(int smooth, int subdiv) {
    if (smooth)
        glShadeModel(GL_SMOOTH);
    else
        glShadeModel(GL_FLAT);

    glBegin(GL_TRIANGLES);
    for (int i = 0; i < 20; i++) {
        GLfloat* v1 = vdata[tindices[i][0]];
        GLfloat* v2 = vdata[tindices[i][1]];
        GLfloat* v3 = vdata[tindices[i][2]];
        if (subdiv)
            subdivide(v1, v2, v3, subdivDepth);
        else
            drawTriangle(v1, v2, v3);
    }
    glEnd();
}

void display() {
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_LIGHTING);
    glEnable(GL_LIGHT0);
    glPolygonMode(GL_FRONT_AND_BACK, polygonMode);

    int w = glutGet(GLUT_WINDOW_WIDTH) / 3;
    int h = glutGet(GLUT_WINDOW_HEIGHT);

    for (int i = 0; i < 3; i++) {
        glViewport(i * w, 0, w, h);
        glMatrixMode(GL_PROJECTION);
        glLoadIdentity();
        gluPerspective(60, (float)w / h, 1, 10);
        glMatrixMode(GL_MODELVIEW);
        glLoadIdentity();
        gluLookAt(0, 0, 3, 0, 0, 0, 0, 1, 0);

        glRotatef(rotateX, 1, 0, 0);
        glRotatef(rotateY, 0, 1, 0);
        glColor3f(1.0, 0.7, 0.1);

        if (i == 0)
            drawIcosahedron(0, 0);
        else if (i == 1)
            drawIcosahedron(1, 0);
        else
            drawIcosahedron(1, 1);
    }

    glutSwapBuffers();
}

void keyboard(unsigned char key, int x, int y) {
    switch (key) {
    case '+':
        if (subdivDepth < 5) subdivDepth++;
        break;
    case '-':
        if (subdivDepth > 0) subdivDepth--;
        break;
    case 'm':
        polygonMode = (polygonMode == GL_FILL) ? GL_LINE : GL_FILL;
        break;
    case 27: // ESC key
        exit(0);
    }
    glutPostRedisplay();
}

void specialKeys(int key, int x, int y) {
    switch (key) {
    case GLUT_KEY_LEFT:
        rotateY -= 5;
        break;
    case GLUT_KEY_RIGHT:
        rotateY += 5;
        break;
    case GLUT_KEY_UP:
        rotateX -= 5;
        break;
    case GLUT_KEY_DOWN:
        rotateX += 5;
        break;
    }
    glutPostRedisplay();
}

int main(int argc, char** argv) {
    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB | GLUT_DEPTH);
    glutInitWindowSize(900, 300);
    glutCreateWindow("Icosahedron: Flat, Smooth, Subdivided");

    glEnable(GL_NORMALIZE);
    glEnable(GL_COLOR_MATERIAL);

    glutDisplayFunc(display);
    glutKeyboardFunc(keyboard);
    glutSpecialFunc(specialKeys);

    glutMainLoop();
    return 0;
}
