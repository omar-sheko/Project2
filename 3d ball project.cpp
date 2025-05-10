#define NOMINMAX
#define GLUT_DISABLE_ATEXIT_HACK
#include <windows.h>
#include <cmath>
#include <GL/glut.h>
#include <iostream>
#include <vector>
#include <string>

// stb_image implementation
#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>

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
Vec3 goalkeeperPos = Vec3(0.0f, 0.5f, -4.0f);
float goalkeeperSpeed = 0.05f;
bool movingRight = true;
int attempts = 3;
int goals = 0;
int level = 1;
float arrowAngle = 0.0f;
float ballSpeed = 0.5f;

float postLeft = -1.5f, postRight = 1.5f, crossbarTop = 1.0f, ground = 0.0f;
Vec3 lightPos = Vec3(0.0f, 5.0f, 0.0f);

GLuint ballTexture, crowdTexture, fieldTexture;

GLuint loadTexture(const char* filename) {
    GLuint textureID;
    glGenTextures(1, &textureID);
    glBindTexture(GL_TEXTURE_2D, textureID);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

    int width, height, nrChannels;
    unsigned char* data = stbi_load(filename, &width, &height, &nrChannels, 0);
    if (data) {
        GLenum format = (nrChannels == 3) ? GL_RGB : GL_RGBA;
        glTexImage2D(GL_TEXTURE_2D, 0, format, width, height, 0, format, GL_UNSIGNED_BYTE, data);
        stbi_image_free(data);
    }
    else {
        cout << "Failed to load texture: " << filename << endl;
        stbi_image_free(data);
        return 0;
    }
    return textureID;
}

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
    glEnable(GL_TEXTURE_2D);
    glBindTexture(GL_TEXTURE_2D, crowdTexture);
    glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);

    // ÎáÝíÉ ÇáÌãÇåíÑ (ÎáÝ ÇáãÑãì)
    glColor3f(1.0f, 1.0f, 1.0f);
    glBegin(GL_QUADS);
    glTexCoord2f(0.0f, 0.0f); glVertex3f(-10.0f, 0.0f, -10.0f);
    glTexCoord2f(1.0f, 0.0f); glVertex3f(10.0f, 0.0f, -10.0f);
    glTexCoord2f(1.0f, 1.0f); glVertex3f(10.0f, 4.0f, -10.0f);
    glTexCoord2f(0.0f, 1.0f); glVertex3f(-10.0f, 4.0f, -10.0f);
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

    // ÎáÝíÉ ÇáÌãÇåíÑ (ÞÏÇã)
    glBegin(GL_QUADS);
    glTexCoord2f(0.0f, 0.0f); glVertex3f(-10.0f, 0.0f, 10.0f);
    glTexCoord2f(1.0f, 0.0f); glVertex3f(10.0f, 0.0f, 10.0f);
    glTexCoord2f(1.0f, 1.0f); glVertex3f(10.0f, 4.0f, 10.0f);
    glTexCoord2f(0.0f, 1.0f); glVertex3f(-10.0f, 4.0f, 10.0f);
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

    // ÎáÝíÉ ÇáÌãÇåíÑ (íãíä)
    glBegin(GL_QUADS);
    glTexCoord2f(0.0f, 0.0f); glVertex3f(10.0f, 0.0f, -10.0f);
    glTexCoord2f(1.0f, 0.0f); glVertex3f(10.0f, 0.0f, 10.0f);
    glTexCoord2f(1.0f, 1.0f); glVertex3f(10.0f, 4.0f, 10.0f);
    glTexCoord2f(0.0f, 1.0f); glVertex3f(10.0f, 4.0f, -10.0f);
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

    // ÎáÝíÉ ÇáÌãÇåíÑ (ÔãÇá)
    glBegin(GL_QUADS);
    glTexCoord2f(0.0f, 0.0f); glVertex3f(-10.0f, 0.0f, -10.0f);
    glTexCoord2f(1.0f, 0.0f); glVertex3f(-10.0f, 0.0f, 10.0f);
    glTexCoord2f(1.0f, 1.0f); glVertex3f(-10.0f, 4.0f, 10.0f);
    glTexCoord2f(0.0f, 1.0f); glVertex3f(-10.0f, 4.0f, -10.0f);
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

    glDisable(GL_TEXTURE_2D);

    GLfloat crowdLight[] = { 0.7f, 0.7f, 1.0f, 1.0f };
    glLightfv(GL_LIGHT1, GL_DIFFUSE, crowdLight);
    GLfloat lightPos2[] = { 0.0f, 4.0f, 0.0f, 1.0f };
    glLightfv(GL_LIGHT1, GL_POSITION, lightPos2);
    glEnable(GL_LIGHT1);
}

void drawRoof() {
    glColor3f(1.0f, 1.0f, 1.0f);
    glBegin(GL_QUADS);
    glVertex3f(-10.0f, 4.2f, -10.0f);
    glVertex3f(10.0f, 4.2f, -10.0f);
    glVertex3f(10.0f, 4.2f, 10.0f);
    glVertex3f(-10.0f, 4.2f, 10.0f);
    glEnd();
}

void drawArrow() {
    if (!ballShot) {
        glPushMatrix();
        glTranslatef(ballPos.x, ballPos.y + 0.2f, ballPos.z);
        glRotatef(arrowAngle, 0.0f, 1.0f, 0.0f);
        glColor3f(1.0f, 0.0f, 0.0f);
        glLineWidth(3.0f);
        glBegin(GL_LINES);
        glVertex3f(0.0f, 0.0f, 0.0f);
        glVertex3f(0.0f, 0.0f, -0.3f);
        glEnd();
        glBegin(GL_TRIANGLES);
        glVertex3f(-0.05f, 0.0f, -0.3f);
        glVertex3f(0.05f, 0.0f, -0. pendantf);
        glVertex3f(0.0f, 0.0f, -0.4f);
        glEnd();
        glPopMatrix();
    }
}

void drawSpeedIndicator() {
    if (!ballShot) {
        glMatrixMode(GL_PROJECTION);
        glPushMatrix();
        glLoadIdentity();
        gluOrtho2D(0, width, 0, height);
        glMatrixMode(GL_MODELVIEW);
        glPushMatrix();
        glLoadIdentity();

        glColor3f(0.3f, 0.3f, 0.3f);
        glBegin(GL_QUADS);
        glVertex2i(10, 50);
        glVertex2i(110, 50);
        glVertex2i(110, 70);
        glVertex2i(10, 70);
        glEnd();

        glColor3f(0.0f, 1.0f, 0.0f);
        glBegin(GL_QUADS);
        glVertex2i(10, 50);
        glVertex2i(10 + (ballSpeed * 100), 50);
        glVertex2i(10 + (ballSpeed * 100), 70);
        glVertex2i(10, 70);
        glEnd();

        glPopMatrix();
        glMatrixMode(GL_PROJECTION);
        glPopMatrix();
        glMatrixMode(GL_MODELVIEW);
    }
}

void drawSphere(float radius) {
    glEnable(GL_TEXTURE_2D);
    glBindTexture(GL_TEXTURE_2D, ballTexture);
    glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);
    glColor3f(1.0f, 1.0f, 1.0f);
    glutSolidSphere(radius, 20, 20);
    glDisable(GL_TEXTURE_2D);
}

void drawCube(float size) {
    glColor3f(0.0f, 0.0f, 1.0f);
    glutSolidCube(size);
}

void drawField() {
    glEnable(GL_TEXTURE_2D);
    glBindTexture(GL_TEXTURE_2D, fieldTexture);
    glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);
    glColor3f(1.0f, 1.0f, 1.0f);
    glBegin(GL_QUADS);
    glTexCoord2f(0.0f, 0.0f); glVertex3f(-10, 0, -10);
    glTexCoord2f(1.0f, 0.0f); glVertex3f(10, 0, -10);
    glTexCoord2f(1.0f, 1.0f); glVertex3f(10, 0, 10);
    glTexCoord2f(0.0f, 1.0f); glVertex3f(-10, 0, 10);
    glEnd();
    glDisable(GL_TEXTURE_2D);

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

    string hud = "Goals: " + to_string(goals) + " Attempts: " + to_string(attempts) + " Level: " + to_string(level);
    glRasterPos2i(10, height - 30);
    for (char c : hud) {
        glutBitmapCharacter(GLUT_BITMAP_HELVETICA_18, c);
    }

    drawSpeedIndicator();

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
    Buchers else {
        goalkeeperPos.x -= goalkeeperSpeed;
        if (ballPos.x <= postLeft + 0.3f) movingRight = true;
    }

    if (ballShot) {
        ballPos = ballPos + ballVelocity;
        ballVelocity.y -= 0.01f;

        if (ballPos.z <= -4.0f) {
            if (ballPos.x >= postLeft && ballPos.x <= postRight && ballPos.y <= crossbarTop && !checkCollision()) {
                goals++;
                if (goals % 5 == 0) {
                    goalkeeperSpeed += 0.02f;
                    level++;
                }
            }
            else {
                attempts--;
            }

            ballShot = false;
            ballPos = Vec3(0.0f, 0.1f, 2.0f);
            ballVelocity = Vec3(0.0f);
            arrowAngle = 0.0f;
            ballSpeed = 0.5f;

            if (attempts <= 0) {
                cout << "Game Over! Final Goals: " << goals << endl;
                exit(0);
            }
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

void mouse(int button, int state, int x, int y) {
    if (button == GLUT_RIGHT_BUTTON && state == GLUT_DOWN && !ballShot && attempts > 0) {
        ballShot = true;
        float angleRad = radians(arrowAngle);
        float speed = 0.1f + (ballSpeed * 0.2f);
        ballVelocity = Vec3(cos(angleRad) * speed, 0.1f, -speed);
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

    if (!ballShot) {
        arrowAngle += xoffset * 0.1f;
        if (arrowAngle > 45.0f) arrowAngle = 45.0f;
        if (arrowAngle < -45.0f) arrowAngle = -45.0f;

        ballSpeed -= yoffset * 0.005f;
        if (ballSpeed > 1.0f) ballSpeed = 1.0f;
        if (ballSpeed < 0.0f) ballSpeed = 0.0f;
    }

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

    GLfloat crowdAmbient[] = { 0.2f, 0.2f, 0.3f, 1.0f };
    glLightfv(GL_LIGHT1, GL_AMBIENT, crowdAmbient);

    ballTexture = loadTexture("ball_texture.png");
    crowdTexture = loadTexture("crowd_texture.jpg");
    fieldTexture = loadTexture("field_texture.jpg");

    glMatrixMode(GL_PROJECTION);
    gluPerspective(45.0f, (float)width / height, 0.1f, 100.0f);
    glMatrixMode(GL_MODELVIEW);
}

int main(int argc, char** argv) {
    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB | GLUT_DEPTH);
    glutInitWindowSize(width, height);
    glutCreateWindow("Penalty Shootout Game with Textures");

    init();

    glutDisplayFunc(display);
    glutKeyboardFunc(keyboard);
    glutMouseFunc(mouse);
    glutMotionFunc(motion);
    glutTimerFunc(0, update, 0);

    glutMainLoop();
    return 0;
}