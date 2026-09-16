#include <GL/freeglut.h>
#include <cmath>
#include <ctime>

#define PI 3.14159265358979323846

// رسم دایره (صفحه ساعت)
void drawCircle(float radius) {
    glBegin(GL_LINE_LOOP);
    for (int i = 0; i < 360; i++) {
        float theta = i * PI / 180.0;
        glVertex2f(radius * cos(theta), radius * sin(theta));
    }
    glEnd();
}

// رسم عقربه
void drawHand(float length, float width, float angle) {
    glLineWidth(width);
    glBegin(GL_LINES);
    glVertex2f(0.0f, 0.0f);
    glVertex2f(length * cos(angle), length * sin(angle));
    glEnd();
}

void display() {
    glClear(GL_COLOR_BUFFER_BIT);

    // رسم صفحه ساعت
    glColor3f(1.0, 1.0, 1.0);
    drawCircle(0.9f);
    
    
    for (int i = 1; i <= 12; i++) {
        float angle = (-i * 30 + 90) * PI / 180.0;  // اصلاح شد
        float x = 0.75 * cos(angle);
        float y = 0.75 * sin(angle);
    
        glRasterPos2f(x, y);
        if(i < 10)
            glutBitmapCharacter(GLUT_BITMAP_HELVETICA_18, '0' + i);
        else {  // برای 10, 11, 12
            glutBitmapCharacter(GLUT_BITMAP_HELVETICA_18, '1');
            glutBitmapCharacter(GLUT_BITMAP_HELVETICA_18, '0' + (i % 10));
        }
    }
    
    // گرفتن زمان سیستم
    time_t now = time(0);
    tm *ltm = localtime(&now);

    int hours = ltm->tm_hour % 12;
    int minutes = ltm->tm_min;
    int seconds = ltm->tm_sec;

    // محاسبه زاویه‌ها
    float secondAngle = (90 - seconds * 6) * PI / 180.0;
    float minuteAngle = (90 - (minutes * 6 + seconds * 0.1)) * PI / 180.0;
    float hourAngle = (90 - (hours * 30 + minutes * 0.5)) * PI / 180.0;

    // رسم عقربه ثانیه (قرمز)
    glColor3f(1.0, 0.0, 0.0);
    drawHand(0.8f, 1.0f, secondAngle);

    // رسم عقربه دقیقه (سبز)
    glColor3f(0.0, 1.0, 0.0);
    drawHand(0.6f, 3.0f, minuteAngle);

    // رسم عقربه ساعت (آبی)
    glColor3f(0.0, 0.0, 1.0);
    drawHand(0.4f, 5.0f, hourAngle);

    glutSwapBuffers();
}

void timer(int value) {
    glutPostRedisplay();             // دوباره رسم کن
    glutTimerFunc(1000, timer, 0);   // هر یک ثانیه
}

int main(int argc, char** argv) {
    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB);
    glutInitWindowSize(600, 600);
    glutCreateWindow("Analog Clock - OpenGL");

    glClearColor(0, 0, 0, 1);  // پس‌زمینه مشکی
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    gluOrtho2D(-1, 1, -1, 1);

    glutDisplayFunc(display);
    glutTimerFunc(0, timer, 0);
    glutMainLoop();
    return 0;
}
