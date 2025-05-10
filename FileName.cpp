#define NOMINMAX
#define GLUT_DISABLE_ATEXIT_HACK
#include <windows.h>
#include <cmath>
#include <GL/glut.h>
#include <iostream>
#include <vector>
#include <string>

using namespace std;

struct Vec3 {
    float x, y, z;
    Vec3(float x = 0.0f, float y = 0.0f, float z = 0.0f) : x(x), y(y), z(z) {}
    Vec3 operator+(const Vec3& other) const { return Vec3(x + other.x, y + other.y, z + other.z); }
    Vec3 operator-(const Vec3& other) const { return Vec3(x - other.x, y - other.y, z - other.z); }
    Vec3 operator*(float scalar) const { return Vec3(x * scalar, y * scalar, z * scalar); }
    float length() const { return sqrt(x * x + y * y + z * z); }
    Vec3 normalize() const { float len = length(); if (len > 0) return Vec3(x / len, y / len, z / len); return *this; }
    Vec3 cross(const Vec3& other) const {
        return Vec3(y * other.z - z * other.y, z * other.x - x * other.z, x * other.y - y * other.x);
    }
};

float radians(float degrees) { return degrees * 3.14159f / 180.0f; }

Vec3 cameraPos = Vec3(0.0f, 1.0f, 5.0f);
Vec3 cameraFront = Vec3(0.0f, 0.0f, -1.0f);
Vec3 cameraUp = Vec3(0.0f, 1.0f, 0.0f);
float yaw = -90.0f, pitch = 0.0f;
int width = 800, height = 600;

Vec3 ballPos = Vec3(0.0f, 0.1f, 2.0f);
Vec3 ballVelocity = Vec3(0.0f);
bool ballShot = false;
Vec3 goalkeeperPos = Vec3(0.0f, 0.1f, -4.0f);
float goalkeeperSpeed = 0.08f; // ”—⁄… «·Õ«—” «·√”«”Ì… (“Ì«œ… „‰ 0.05 ≈·Ï 0.08)
bool movingRight = true;
int attempts = 3;
int goals = 0;
int level = 1;
float aimX = 0.0f, aimY = 0.5f;
float arrowSpeed = 0.05f;
bool arrowMoving = true;
bool arrowDirectionRight = true;
float ballSpeed = 0.0f; // „ €Ì— · Œ“Ì‰ ”—⁄… «·ﬂ—… ⁄‰œ «· ”œÌœ

float postLeft = -1.5f, postRight = 1.5f, crossbarTop = 1.0f, ground = 0.0f;
Vec3 lightPos = Vec3(0.0f, 5.0f, 0.0f);

void drawFan(float x, float y, float z) {
    glPushMatrix();
    glTranslatef(x, y, z);

    glColor3f(1.0f, 0.8f, 0.6f);
    glutSolidSphere(0.12f, 10, 10);

    glColor3f(0.9f, 0.1f, 0.1f);
    glTranslatef(0.0f, -0.22f, 0.0f);
    glutSolidCone(0.15f, 0.45f, 10, 10);

    glPopMatrix();
}

void drawCrowd() {
    glColor3f(0.1f, 0.1f, 0.3f);
    glBegin(GL_QUADS);
    glVertex3f(-10.0f, 0.0f, -10.0f);
    glVertex3f(10.0f, 0.0f, -10.0f);
    glVertex3f(10.0f, 4.0f, -10.0f);
    glVertex3f(-10.0f, 4.0f, -10.0f);
    glEnd();

    for (float x = -9.5f; x <= 9.5f; x += 0.6f) {
        for (float y = 0.5f; y <= 3.5f; y += 0.6f) {
            int color = (int)(x * 10 + y * 10) % 3;
            if (color == 0) glColor3f(0.9f, 0.1f, 0.1f);
            else if (color == 1) glColor3f(0.1f, 0.9f, 0.1f);
            else glColor3f(0.1f, 0.1f, 0.9f);
            drawFan(x, y, -9.9f);
        }
    }

    glColor3f(0.1f, 0.1f, 0.3f);
    glBegin(GL_QUADS);
    glVertex3f(-10.0f, 0.0f, 10.0f);
    glVertex3f(10.0f, 0.0f, 10.0f);
    glVertex3f(10.0f, 4.0f, 10.0f);
    glVertex3f(-10.0f, 4.0f, 10.0f);
    glEnd();

    for (float x = -9.5f; x <= 9.5f; x += 0.6f) {
        for (float y = 0.5f; y <= 3.5f; y += 0.6f) {
            int color = (int)(x * 10 + y * 10) % 3;
            if (color == 0) glColor3f(0.9f, 0.1f, 0.1f);
            else if (color == 1) glColor3f(0.1f, 0.9f, 0.1f);
            else glColor3f(0.1f, 0.1f, 0.9f);
            drawFan(x, y, 9.9f);
        }
    }

    glColor3f(0.1f, 0.1f, 0.3f);
    glBegin(GL_QUADS);
    glVertex3f(10.0f, 0.0f, -10.0f);
    glVertex3f(10.0f, 0.0f, 10.0f);
    glVertex3f(10.0f, 4.0f, 10.0f);
    glVertex3f(10.0f, 4.0f, -10.0f);
    glEnd();

    for (float z = -9.5f; z <= 9.5f; z += 0.6f) {
        for (float y = 0.5f; y <= 3.5f; y += 0.6f) {
            int color = (int)(z * 10 + y * 10) % 3;
            if (color == 0) glColor3f(0.9f, 0.1f, 0.1f);
            else if (color == 1) glColor3f(0.1f, 0.9f, 0.1f);
            else glColor3f(0.1f, 0.1f, 0.9f);
            drawFan(9.9f, y, z);
        }
    }

    glColor3f(0.1f, 0.1f, 0.3f);
    glBegin(GL_QUADS);
    glVertex3f(-10.0f, 0.0f, -10.0f);
    glVertex3f(-10.0f, 0.0f, 10.0f);
    glVertex3f(-10.0f, 4.0f, 10.0f);
    glVertex3f(-10.0f, 4.0f, -10.0f);
    glEnd();

    for (float z = -9.5f; z <= 9.5f; z += 0.6f) {
        for (float y = 0.5f; y <= 3.5f; y += 0.6f) {
            int color = (int)(z * 10 + y * 10) % 3;
            if (color == 0) glColor3f(0.9f, 0.1f, 0.1f);
            else if (color == 1) glColor3f(0.1f, 0.9f, 0.1f);
            else glColor3f(0.1f, 0.1f, 0.9f);
            drawFan(-9.9f, y, z);
        }
    }

    GLfloat crowdLight[] = { 0.7f, 0.7f, 1.0f, 1.0f };
    glLightfv(GL_LIGHT1, GL_DIFFUSE, crowdLight);
    GLfloat lightPos2[] = { 0.0f, 4.0f, 0.0f, 1.0f };
    glLightfv(GL_LIGHT1, GL_POSITION, lightPos2);
    glEnable(GL_LIGHT1);
}

void drawRoof() {
    glColor3f(0.2f, 0.2f, 0.3f);
    glBegin(GL_QUADS);
    glVertex3f(-10.0f, 4.2f, -10.0f);
    glVertex3f(10.0f, 4.2f, -10.0f);
    glVertex3f(10.0f, 4.2f, 10.0f);
    glVertex3f(-10.0f, 4.2f, 10.0f);
    glEnd();
}

void drawSphere(float radius) {
    glColor3f(1.0f, 0.5f, 0.0f);
    glutSolidSphere(radius, 20, 20);
}

void drawCube(float size) {
    glColor3f(0.0f, 0.0f, 1.0f);
    glutSolidCube(size);
}

void drawField() {
    glColor3f(0.0f, 0.6f, 0.0f);
    glBegin(GL_QUADS);
    glVertex3f(-10, 0, -10);
    glVertex3f(10, 0, -10);
    glVertex3f(10, 0, 10);
    glVertex3f(-10, 0, 10);
    glEnd();

    glColor3f(1.0f, 1.0f, 1.0f);
    glLineWidth(2.0f);
    glBegin(GL_LINE_LOOP);
    glVertex3f(-5.0f, 0.01f, -3.0f);
    glVertex3f(5.0f, 0.01f, -3.0f);
    glVertex3f(5.0f, 0.01f, 3.0f);
    glVertex3f(-5.0f, 0.01f, 3.0f);
    glEnd();
}

void drawGoalpost() {
    glColor3f(1.0f, 1.0f, 1.0f);

    glPushMatrix();
    glTranslatef(postLeft, crossbarTop / 2, -4.0f);
    glScalef(0.1f, crossbarTop, 0.1f);
    glutSolidCube(1.0f);
    glPopMatrix();

    glPushMatrix();
    glTranslatef(postRight, crossbarTop / 2, -4.0f);
    glScalef(0.1f, crossbarTop, 0.1f);
    glutSolidCube(1.0f);
    glPopMatrix();

    glPushMatrix();
    glTranslatef(0.0f, crossbarTop, -4.0f);
    glScalef(postRight - postLeft, 0.1f, 0.1f);
    glutSolidCube(1.0f);
    glPopMatrix();

    glColor3f(0.8f, 0.8f, 0.8f);
    glLineWidth(1.0f);
    glBegin(GL_LINES);
    for (float y = 0.1f; y < crossbarTop; y += 0.2f) {
        glVertex3f(postLeft, y, -4.0f);
        glVertex3f(postRight, y, -4.0f);
    }
    for (float x = postLeft; x < postRight; x += 0.2f) {
        glVertex3f(x, 0.1f, -4.0f);
        glVertex3f(x, crossbarTop, -4.0f);
    }
    glEnd();
}

void drawArrow() {
    if (!ballShot && attempts > 0) {
        Vec3 targetPos(aimX, aimY, -4.0f);
        Vec3 arrowPos = ballPos + Vec3(0.0f, 0.0f, -0.5f);
        Vec3 direction = (targetPos - arrowPos).normalize();

        float angleY = atan2(direction.x, direction.z) * 180.0f / 3.14159f;
        float angleX = asin(direction.y) * 180.0f / 3.14159f;

        glPushMatrix();
        glTranslatef(arrowPos.x, arrowPos.y, arrowPos.z);
        glRotatef(angleY, 0.0f, 1.0f, 0.0f);
        glRotatef(-angleX, 1.0f, 0.0f, 0.0f);

        // Ã”„ «·”Â„ (√”ÿÊ«‰… √ﬂ»—)
        glColor3f(0.5f, 0.5f, 0.5f);
        glPushMatrix();
        glRotatef(90.0f, 1.0f, 0.0f, 0.0f);
        GLUquadric* quad = gluNewQuadric();
        gluCylinder(quad, 0.03f, 0.03f, 0.8f, 12, 12); // “Ì«œ… «·ÕÃ„ (0.015->0.03, 0.4->0.8)
        gluDeleteQuadric(quad);
        glPopMatrix();

        // —√” «·”Â„ („Œ—Êÿ √ﬂ»—)
        glColor3f(1.0f, 0.0f, 0.0f);
        glPushMatrix();
        glTranslatef(0.0f, 0.0f, 0.8f);
        glRotatef(90.0f, 1.0f, 0.0f, 0.0f);
        glutSolidCone(0.1f, 0.3f, 12, 12); // “Ì«œ… «·ÕÃ„ (0.05->0.1, 0.15->0.3)
        glPopMatrix();

        // “⁄«‰› «·–Ì· (√ﬂ»—)
        glColor3f(0.5f, 0.5f, 0.5f);
        glPushMatrix();
        glTranslatef(0.0f, 0.0f, 0.2f); //  ⁄œÌ· «·„Êﬁ⁄ ·Ì‰«”» «·”Â„ «·√ﬂ»—
        for (int i = 0; i < 4; i++) {
            glPushMatrix();
            glRotatef(i * 90.0f, 0.0f, 0.0f, 1.0f);
            glBegin(GL_TRIANGLES);
            glVertex3f(0.0f, 0.0f, 0.0f);
            glVertex3f(0.1f, 0.0f, 0.1f); // “Ì«œ… «·ÕÃ„ (0.05->0.1)
            glVertex3f(-0.1f, 0.0f, 0.1f);
            glEnd();
            glPopMatrix();
        }
        glPopMatrix();

        glPopMatrix();
    }
}

void shootBall() {
    if (!ballShot && attempts > 0) {
        ballShot = true;
        Vec3 target(aimX, aimY, -4.0f);
        Vec3 direction = (target - ballPos).normalize();
        float shotSpeed = 0.5f; // ”—⁄… «· ”œÌœ
        ballVelocity = direction * shotSpeed;
        ballSpeed = shotSpeed; //  Œ“Ì‰ ”—⁄… «·ﬂ—… ·⁄—÷Â«
        arrowMoving = false;
    }
}

bool checkCollision() {
    float distX = ballPos.x - goalkeeperPos.x;
    float distY = ballPos.y - goalkeeperPos.y;
    float distZ = ballPos.z - goalkeeperPos.z;
    float distance = sqrt(distX * distX + distY * distY + distZ * distZ);
    if (distance < 0.5f) return true;
    if (ballPos.z <= -4.0f && (ballPos.x < postLeft || ballPos.x > postRight || ballPos.y > crossbarTop)) return true;
    return false;
}

void display() {
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glLoadIdentity();

    gluLookAt(cameraPos.x, cameraPos.y, cameraPos.z,
        cameraPos.x + cameraFront.x, cameraPos.y + cameraFront.y, cameraPos.z + cameraFront.z,
        cameraUp.x, cameraUp.y, cameraUp.z);

    glEnable(GL_LIGHTING);
    glEnable(GL_LIGHT0);
    GLfloat light_pos[] = { lightPos.x, lightPos.y, lightPos.z, 1.0f };
    glLightfv(GL_LIGHT0, GL_POSITION, light_pos);

    drawField();
    drawCrowd();
    drawGoalpost();
    drawRoof();

    glPushMatrix();
    glTranslatef(goalkeeperPos.x, goalkeeperPos.y, goalkeeperPos.z);
    glScalef(0.3f, 0.5f, 0.3f);
    drawCube(1.0f);
    glPopMatrix();

    glPushMatrix();
    glTranslatef(ballPos.x, ballPos.y, ballPos.z);
    drawSphere(0.1f);
    glPopMatrix();

    drawArrow();

    glDisable(GL_LIGHTING);

    glMatrixMode(GL_PROJECTION);
    glPushMatrix();
    glLoadIdentity();
    gluOrtho2D(0, width, 0, height);
    glMatrixMode(GL_MODELVIEW);
    glPushMatrix();
    glLoadIdentity();
    glColor3f(1, 1, 1);

    // ⁄—÷ „⁄·Ê„«  «··⁄»…
    string hud = "Goals: " + to_string(goals) + " Attempts: " + to_string(attempts) + " Level: " + to_string(level);
    glRasterPos2i(10, height - 30);
    for (char c : hud) {
        glutBitmapCharacter(GLUT_BITMAP_HELVETICA_18, c);
    }

    // ⁄—÷ ”—⁄… «·ﬂ—…
    string speedText = "Ball Speed: " + to_string((int)(ballSpeed * 100)) + " units";
    glRasterPos2i(10, height - 60);
    for (char c : speedText) {
        glutBitmapCharacter(GLUT_BITMAP_HELVETICA_18, c);
    }

    glPopMatrix();
    glMatrixMode(GL_PROJECTION);
    glPopMatrix();
    glMatrixMode(GL_MODELVIEW);

    glutSwapBuffers();
}

void update(int value) {
    if (movingRight) {
        goalkeeperPos.x += goalkeeperSpeed;
        if (goalkeeperPos.x >= postRight - 0.3f) movingRight = false;
    }
    else {
        goalkeeperPos.x -= goalkeeperSpeed;
        if (goalkeeperPos.x <= postLeft + 0.3f) movingRight = true;
    }

    if (arrowMoving && !ballShot && attempts > 0) {
        if (arrowDirectionRight) {
            aimX += arrowSpeed;
            if (aimX >= postRight) {
                aimX = postRight;
                arrowDirectionRight = false;
            }
        }
        else {
            aimX -= arrowSpeed;
            if (aimX <= postLeft) {
                aimX = postLeft;
                arrowDirectionRight = true;
            }
        }
    }

    if (ballShot) {
        ballPos = ballPos + ballVelocity;
        ballVelocity.y -= 0.01f;

        if (ballPos.z <= -4.0f) {
            if (ballPos.x >= postLeft && ballPos.x <= postRight && ballPos.y <= crossbarTop && !checkCollision()) {
                goals++;
                goalkeeperSpeed += 0.03f; // “Ì«œ… ”—⁄… «·Õ«—” „⁄ ﬂ· Âœ› (“Ì«œ… „‰ 0.02 ≈·Ï 0.03)
                level++;
            }
            else {
                attempts--;
            }

            ballShot = false;
            ballPos = Vec3(0.0f, 0.1f, 2.0f);
            ballVelocity = Vec3(0.0f);
            aimX = 0.0f;
            aimY = 0.5f;
            arrowMoving = true;
            ballSpeed = 0.0f; // ≈⁄«œ… ÷»ÿ ”—⁄… «·ﬂ—…

            if (attempts <= 0) {
                cout << "Game Over! Final Goals: " << goals << endl;
                exit(0);
            }
        }

        if (ballPos.y < 0.1f) {
            ballPos.y = 0.1f;
            ballVelocity.y = -ballVelocity.y * 0.5f;
        }
    }

    glutPostRedisplay();
    glutTimerFunc(16, update, 0);
}

void keyboard(unsigned char key, int x, int y) {
    float cameraSpeed = 0.1f;

    switch (key) {
    case 27: exit(0); break;
    case 'w': cameraPos = cameraPos + (cameraFront * cameraSpeed); break;
    case 's': cameraPos = cameraPos - (cameraFront * cameraSpeed); break;
    case 'a': cameraPos = cameraPos - (cameraFront.cross(cameraUp).normalize() * cameraSpeed); break;
    case 'd': cameraPos = cameraPos + (cameraFront.cross(cameraUp).normalize() * cameraSpeed); break;
    }
}

void specialKeys(int key, int x, int y) {
}

void mouse(int button, int state, int x, int y) {
    if (button == GLUT_LEFT_BUTTON && state == GLUT_DOWN) {
        shootBall();
    }
}

void motion(int x, int y) {
    static bool firstMouse = true;
    static float lastX = width / 2, lastY = height / 2;

    if (firstMouse) {
        lastX = x;
        lastY = y;
        firstMouse = false;
    }

    float xoffset = x - lastX;
    float yoffset = lastY - y;
    lastX = x;
    lastY = y;

    float sensitivity = 0.1f;
    xoffset *= sensitivity;
    yoffset *= sensitivity;

    yaw += xoffset;
    pitch += yoffset;

    if (pitch > 89.0f) pitch = 89.0f;
    if (pitch < -89.0f) pitch = -89.0f;

    Vec3 direction;
    direction.x = cos(radians(yaw)) * cos(radians(pitch));
    direction.y = sin(radians(pitch));
    direction.z = sin(radians(yaw)) * cos(radians(pitch));
    cameraFront = direction.normalize();
}

void init() {
    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_LIGHTING);
    glEnable(GL_LIGHT0);
    glEnable(GL_COLOR_MATERIAL);
    glEnable(GL_LIGHT1);

    GLfloat lightAmbient[] = { 0.2f, 0.2f, 0.2f, 1.0f };
    GLfloat lightDiffuse[] = { 0.8f, 0.8f, 0.8f, 1.0f };
    GLfloat lightSpecular[] = { 1.0f, 1.0f, 1.0f, 1.0f };
    glLightfv(GL_LIGHT0, GL_AMBIENT, lightAmbient);
    glLightfv(GL_LIGHT0, GL_DIFFUSE, lightDiffuse);
    glLightfv(GL_LIGHT0, GL_SPECULAR, lightSpecular);

    GLfloat crowdAmbient[] = { 0.2f, 0.2f, 0.3f, 1.0f };
    glLightfv(GL_LIGHT1, GL_AMBIENT, crowdAmbient);

    glMatrixMode(GL_PROJECTION);
    gluPerspective(45.0f, (float)width / height, 0.1f, 100.0f);
    glMatrixMode(GL_MODELVIEW);
}

int main(int argc, char** argv) {
    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB | GLUT_DEPTH);
    glutInitWindowSize(width, height);
    glutCreateWindow("Penalty Shootout Game with Auto-Aiming Arrow");

    init();

    glutDisplayFunc(display);
    glutKeyboardFunc(keyboard);
    glutSpecialFunc(specialKeys);
    glutMouseFunc(mouse);
    glutMotionFunc(motion);
    glutTimerFunc(0, update, 0);

    glutMainLoop();
    return 0;
}