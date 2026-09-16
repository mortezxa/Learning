// main.cpp
#include <GL/freeglut.h>
#include <cmath>
#include <ctime>
#include <chrono>

#define PI 3.14159265358979323846

//  تنظیمات انیمیشن شروع 
float startupProgress = 0.0f;
bool animationDone = false;

// تنظیمات گرافیکی
void enableBlend() {
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
}

// رسم نوار دایره
void drawCircle(float radius) {
    glBegin(GL_LINE_LOOP);
    for (int i = 0; i < 360; ++i) {
        float theta = i * PI / 180.0f;
        glVertex2f(radius * cosf(theta), radius * sinf(theta));
    }
    glEnd();
}

// رسم دایره پرشده
void fillCircle(float radius) {
    glBegin(GL_TRIANGLE_FAN);
    glVertex2f(0.0f, 0.0f);
    for (int i = 0; i <= 360; ++i) {
        float theta = i * PI / 180.0f;
        glVertex2f(radius * cosf(theta), radius * sinf(theta));
    }
    glEnd();
}

// تیک‌ها
void drawTicks() {
    for (int i = 0; i < 60; ++i) {
        float angle = i * 6.0f * PI / 180.0f;
        float inner = (i % 5 == 0) ? 0.85f : 0.88f;
        float outer = 0.9f;

        float x1 = inner * cosf(angle), y1 = inner * sinf(angle);
        float x2 = outer * cosf(angle), y2 = outer * sinf(angle);

        glLineWidth((i % 5 == 0) ? 3.0f : 1.0f);
        glBegin(GL_LINES);
        glVertex2f(x1, y1);
        glVertex2f(x2, y2);
        glEnd();
    }
}

// اعداد رومی
void drawNumbers() {
    const char* roman[12] = {
        "I","II","III","IV","V","VI",
        "VII","VIII","IX","X","XI","XII"
    };
    for (int i = 0; i < 12; ++i) {
        float angle = (-(i + 1) * 30.0f + 90.0f) * PI / 180.0f;
        float x = 0.75f * cosf(angle);
        float y = 0.75f * sinf(angle);
        glColor3f(0.5f, 0.5f, 0.5f);
        glRasterPos2f(x - 0.03f, y - 0.03f);
        const char* p = roman[i];
        while (*p) glutBitmapCharacter(GLUT_BITMAP_HELVETICA_18, *p++);
    }
}

// متون ROLEX / QUARTZ
void drawROLEX() {
    const char* s = "ROLEX";
    glColor3f(0.5f, 0.5f, 0.5f);
    glRasterPos2f(-0.08f, 0.32f);
    const char* p = s;
    while (*p) glutBitmapCharacter(GLUT_BITMAP_HELVETICA_18, *p++);
}
void drawQUARTZ() {
    const char* s = "QUARTZ";
    glColor3f(0.5f, 0.5f, 0.5f);
    glRasterPos2f(-0.05f, 0.27f);
    const char* p = s;
    while (*p) glutBitmapCharacter(GLUT_BITMAP_HELVETICA_10, *p++);
}

//  سایه و مثلث برجسته برای عقربه 
void drawHandWithShadow(float length, float width, float angle, float r, float g, float b) {
    float tipX = length * cosf(angle);
    float tipY = length * sinf(angle);
    float backLen = 0.06f;
    float baseX = -backLen * cosf(angle);
    float baseY = -backLen * sinf(angle);
    float halfW = width / 300.0f;
    float perpX =  halfW * sinf(angle);
    float perpY = -halfW * cosf(angle);

    float leftX  = baseX + perpX;
    float leftY  = baseY + perpY;
    float rightX = baseX - perpX;
    float rightY = baseY - perpY;

    // سایه
    float shadowOffsetX = 0.02f;
    float shadowOffsetY = -0.02f;
    glColor4f(0.0f, 0.0f, 0.0f, 0.28f);
    glBegin(GL_TRIANGLES);
    glVertex2f(tipX + shadowOffsetX, tipY + shadowOffsetY);
    glVertex2f(leftX + shadowOffsetX, leftY + shadowOffsetY);
    glVertex2f(rightX + shadowOffsetX, rightY + shadowOffsetY);
    glEnd();

    // خود عقربه
    glColor3f(r, g, b);
    glBegin(GL_TRIANGLES);
    glVertex2f(tipX, tipY);
    glVertex2f(leftX, leftY);
    glVertex2f(rightX, rightY);
    glEnd();

    // هایلایت
    float highlightScale = 0.92f;
    float hTipX = tipX * highlightScale;
    float hTipY = tipY * highlightScale;
    float hLeftX = leftX * highlightScale;
    float hLeftY = leftY * highlightScale;
    float hRightX = rightX * highlightScale;
    float hRightY = rightY * highlightScale;

    float hr = fminf(r + 0.18f, 1.0f);
    float hg = fminf(g + 0.18f, 1.0f);
    float hb = fminf(b + 0.18f, 1.0f);
    glColor4f(hr, hg, hb, 0.35f);
    glBegin(GL_TRIANGLES);
    glVertex2f(hTipX, hTipY);
    glVertex2f(hLeftX, hLeftY);
    glVertex2f(hRightX, hRightY);
    glEnd();
}

// قاب 
void drawBezelWithEmboss() {
    enableBlend();
    glColor4f(0.0f, 0.0f, 0.0f, 0.25f);
    glBegin(GL_TRIANGLE_FAN);
    glVertex2f(0.02f, -0.02f);
    for (int i = 0; i <= 360; ++i) {
        float th = i * PI / 180.0f;
        glVertex2f(0.94f * cosf(th) + 0.02f, 0.94f * sinf(th) - 0.02f);
    }
    glEnd();

    glColor3f(1.0f, 1.0f, 1.0f);
    glLineWidth(3.5f);
    drawCircle(0.905f);

    glColor3f(0.7f, 0.7f, 0.7f);
    glLineWidth(3.0f);
    drawCircle(0.88f);
}

//زمان دقیق 
void getPreciseTime(int &outH12, int &outM, float &outSecFrac) {
    using namespace std::chrono;
    auto now = system_clock::now();
    auto ms = duration_cast<milliseconds>(now.time_since_epoch()).count();
    time_t sec = ms / 1000;
    int msPart = ms % 1000;
    tm local = *std::localtime(&sec);
    outH12 = local.tm_hour % 12;
    outM = local.tm_min;
    outSecFrac = local.tm_sec + msPart / 1000.0f;
}

// نمایش 
void display() {
    glClear(GL_COLOR_BUFFER_BIT);
    enableBlend();

    float alpha = (startupProgress < 1.0f) ? startupProgress : 1.0f;

    // پس‌زمینه
    glColor3f(0.8f * alpha, 0.8f * alpha, 0.8f * alpha);
    glBegin(GL_QUADS);
    glVertex2f(-1,-1); glVertex2f(1,-1); glVertex2f(1,1); glVertex2f(-1,1);
    glEnd();

    // قاب
    if (startupProgress > 0.2f) {
        drawBezelWithEmboss();
    }

    // صفحه داخلی
    if (startupProgress > 0.3f) {
        glColor3f(0.0f, 0.0f, 0.0f);
        fillCircle(0.9f);
    }

    // تیک‌ها و اعداد
    if (startupProgress > 0.5f) {
        drawTicks();
        drawNumbers();
    }

    // زمان
    int h12, m;
    float secFrac;
    getPreciseTime(h12, m, secFrac);

    float preciseSeconds = secFrac;
    float preciseMinutes = m + preciseSeconds / 60.0f;
    float preciseHours = h12 + preciseMinutes / 60.0f;

    float secondAngle = (90.0f - preciseSeconds * 6.0f) * PI / 180.0f;
    float minuteAngle = (90.0f - preciseMinutes * 6.0f) * PI / 180.0f;
    float hourAngle   = (90.0f - preciseHours * 30.0f) * PI / 180.0f;

    // متون
    if (startupProgress > 0.7f) {
        drawROLEX();
        drawQUARTZ();
    }

    // عقربه‌ها
    if (startupProgress > 0.8f) {
        drawHandWithShadow(0.4f, 6.0f, hourAngle, 0.5f, 0.5f, 0.5f);
    }
    if (startupProgress > 0.9f) {
        drawHandWithShadow(0.6f, 4.0f, minuteAngle, 0.5f, 0.5f, 0.5f);
    }
    if (startupProgress > 1.0f) {
        drawHandWithShadow(0.8f, 2.0f, secondAngle, 0.5f, 0.5f, 0.5f);
    }

    // دایره وسط
    if (startupProgress > 0.9f) {
        glColor3f(0.0f, 0.0f, 0.0f);
        fillCircle(0.03f);
    }

    glutSwapBuffers();
}

// تایمر
void timerFunc(int) {
    if (!animationDone) {
        startupProgress += 0.02f;
        if (startupProgress >= 1.2f) {
            startupProgress = 1.2f;
            animationDone = true;
        }
    }
    glutPostRedisplay();
    glutTimerFunc(16, timerFunc, 0);
}

// تغییر اندازه
void reshape(int w, int h) {
    if (h == 0) h = 1;
    glViewport(0,0,w,h);
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    float aspect = (float)w / (float)h;
    if (w <= h) gluOrtho2D(-1.0,1.0,-1.0/aspect,1.0/aspect);
    else           gluOrtho2D(-1.0*aspect,1.0*aspect,-1.0,1.0);
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
}

int main(int argc, char** argv) {
    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGBA);
    glutInitWindowSize(600, 600);
    glutCreateWindow("Analog Clock - Startup Animation + Emboss");

    glClearColor(0,0,0,1);
    enableBlend();

    glutDisplayFunc(display);
    glutReshapeFunc(reshape);
    glutTimerFunc(0, timerFunc, 0);
    glutMainLoop();
    return 0;
}
