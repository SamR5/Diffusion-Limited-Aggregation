
/*

    Brownian Motion Snowflake

*/


#include <GL/gl.h>
#include <GL/glut.h>

#include <iostream>
#include <cstdio>
#include <chrono>
#include <cmath>
#include <random>
#include <ctime>

#define WIDTH 600
#define HEIGHT 600
#define FPS 5

#define PI 3.1415926
#define TWO_PI PI*2
#define PI2 PI/2
#define PI3 PI/3

#define MAX_PARTICLE 200
#define MAX_SIMULTANEOUS 1
#define DOT_RADIUS 4
#define SPEED 1
#define OVERLAP_TOL 0 // in pixels
#define COLLISION_DISTANCE DOT_RADIUS*2 - OVERLAP_TOL


struct Particle {
    float radius;
    float angle;
    bool isFixed;
    void update() {
        radius -= SPEED;
        double a = PI3 * (rand()%100) / 3500;
        angle += (rand()%2) ? -a : a;
    }
    void border_control() {
        if (angle > PI/6)
            angle = PI/6;
        else if (angle < -PI/6)
            angle = -PI/6;

    }
};


bool is_collision(Particle& A, Particle& B);
void init_particles ();
void add_new_particle();
void check_collisions();
void update_particles();
void draw_particle(Particle& p);


Particle allParticles[MAX_PARTICLE];

int currentTotalParticles(0); // current amount of particles in screen

bool is_collision(Particle& A, Particle& B) {
    double AxBx = A.radius * std::cos(A.angle) - B.radius * std::cos(B.angle); // A.x - B.x;
    double AyBy = A.radius * std::sin(A.angle) - B.radius * std::sin(B.angle); // A.y - B.y;

    if (AxBx > COLLISION_DISTANCE)
        return false;
    else if (AyBy > COLLISION_DISTANCE)
        return false;

    float dst(std::sqrt(AxBx*AxBx + AyBy*AyBy));
    return dst < COLLISION_DISTANCE;
}

void init_particles () {
    allParticles[0] = Particle {0, 0, true};
    //allParticles[0] = Particle {WIDTH/2, HEIGHT/2, true};

    currentTotalParticles++;
    for (int i=0; i<MAX_SIMULTANEOUS; i++) {
        add_new_particle();
    }
}

void add_new_particle() {
    if (currentTotalParticles >= MAX_PARTICLE)
        return;
    //double angle(rand()%31);
    Particle P = {WIDTH, 0, false};
    //Particle P = {WIDTH, HEIGHT/2, false};
    allParticles[currentTotalParticles] = P;
    currentTotalParticles++;
}

void check_collisions() {
    // i for the free and j for the fixed
    for (int i=0; i<currentTotalParticles; i++) {
        if (allParticles[i].isFixed == true)
            continue;
        for (int j=0; j<currentTotalParticles; j++) {
            if (allParticles[j].isFixed == false)
                continue;
            if (is_collision(allParticles[i], allParticles[j])) {
                allParticles[i].isFixed = true;
                add_new_particle();
                break;
            }
        }
    }
}

void update_particles() {
    for (int i=0; i<currentTotalParticles; i++) {
        if (allParticles[i].isFixed)
            continue;
        allParticles[i].update();
        allParticles[i].border_control();
    }
}

void draw_walls() {
    glLineWidth(1);
    glColor3f(0.6, 0.6, 0.6);
    for (float x=PI/6; x<TWO_PI; x+=PI3) {
        glBegin(GL_LINE_STRIP);
          glVertex2f(0, 0);
          glVertex2f(WIDTH*std::cos(x), HEIGHT*std::sin(x));
        glEnd();
    }
}

void draw_all() {
    glLineWidth(10.0);
    glColor3f(1.0, 0.0, 0.0);
    Particle p1, p2;
    for (int i=0; i<currentTotalParticles; i++) {
        p1 = allParticles[i];
        for (int j=0; j<currentTotalParticles; j++) {
            if (i==j)
                continue;
            p2 = allParticles[j];
            // drawing the ball
            for (float d=0, e=1; d<TWO_PI; d+=PI3, e*=-1) {
                glBegin(GL_POLYGON);
                for (float i=0; i<TWO_PI; i+=PI/10) {
                    glVertex2f(p1.radius*std::cos(e*p1.angle+d) + (DOT_RADIUS-2)*std::cos(i),
                               p1.radius*std::sin(e*p1.angle+d) + (DOT_RADIUS-2)*std::sin(i));
                }
                glEnd();
            }
            // if collision draw a line between
            if (is_collision(p1, p2)) {
                for (float d=0, e=1; d<TWO_PI; d+=PI3, e*=-1) {
                    glBegin(GL_LINE_STRIP);
                      glVertex2f(p1.radius*std::cos(e*p1.angle+d),
                                 p1.radius*std::sin(e*p1.angle+d));
                      glVertex2f(p2.radius*std::cos(e*p2.angle+d),
                                 p2.radius*std::sin(e*p2.angle+d));
                    glEnd();
                }
                break;
            }
        }
    }
}

void init() {
    glClearColor(0.3, 0.5, 0.3, 1.0);
    std::srand(std::time(0));
    init_particles();
}

void display_callback() {
    glClear (GL_COLOR_BUFFER_BIT);
    glColor3f(1.0, 1.0, 0.5);

    draw_walls();
    draw_all();

    glFlush();
    glutSwapBuffers();
}

void reshape_callback(int width, int height) {
    glViewport(0, 0, (GLsizei)width, (GLsizei) height);
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    int margin(0);
    glOrtho(-WIDTH/2-margin, WIDTH/2+margin,
            -HEIGHT/2-margin, HEIGHT/2+margin,
            -1.0, 0.0);
    glMatrixMode(GL_MODELVIEW);
}

void timer_callback(int) {
    //auto start(std::chrono::steady_clock::now());
    if (currentTotalParticles>=MAX_PARTICLE) {
        currentTotalParticles = 0;
        for (int i=0; i<MAX_PARTICLE; i++) {
            allParticles[i] = Particle();
        }
        init();
    }
    while(currentTotalParticles < MAX_PARTICLE) {
        check_collisions();
        update_particles();
    }
    glutPostRedisplay(); // run the display_callback function

    //auto stop(std::chrono::steady_clock::now());
    //auto duration(std::chrono::duration_cast<std::chrono::milliseconds>(stop-start));
    glutTimerFunc(1000, timer_callback, 0);
}


int main(int argc, char **argv) {
    glutInit(&argc, argv); // initialize
    glutInitDisplayMode(GLUT_RGB | GLUT_DOUBLE);
    glutInitWindowPosition(15, 15); // optional
    glutInitWindowSize(WIDTH, HEIGHT);
    glutCreateWindow("Brownian Snowflake");
    glutDisplayFunc(display_callback);
    glutReshapeFunc(reshape_callback);
    glutTimerFunc(1000/FPS, timer_callback, 0);
    init();
    glutMainLoop();

    return 0;
}
