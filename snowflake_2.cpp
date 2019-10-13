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
#define FPS 50

#define PI 3.1415926
#define TWO_PI 2*PI
#define PI3 PI/3
#define PI4 PI/4
#define MAX_PARTICLE 3000
#define MAX_SIMULTANEOUS 1
#define DOT_RADIUS 1
#define OVERLAP_TOL 0 // in pixels
#define COLLISION_DISTANCE DOT_RADIUS*2-OVERLAP_TOL


struct Particle {
    float x;
    float y;
    bool isFixed;
    void update() {
        //x -= 1;
        x += (rand()%3) ? -1 : 1;
        y += (rand()%2) ? -1 : 1;
    }
    double get_angle() {
        double dst = std::sqrt(x*x+y*y);
        return (y>0) ? std::acos(x / dst) : - std::acos(x / dst);
    }
    void border_control() {
        if (get_angle()>PI/6) {
            y--;
        }
        else if (get_angle()<-PI/6) {
            y++;
        }
        if (x > WIDTH/2) {
            x--;
        }
    }
};


bool is_collision(Particle* A, Particle* B);
void init_particles ();
void delete_particles();
void add_new_particle();
void check_collisions();
void update_particles();
void draw_particle(Particle* p);


Particle** allParticles = new Particle* [MAX_PARTICLE];

int currentTotalParticles(0); // current amount of particles in screen

bool is_collision(Particle* A, Particle* B) {
    double AxBx = std::abs(A->x - B->x);
    double AyBy = std::abs(A->y - B->y);
    if (AxBx > COLLISION_DISTANCE)
        return false;
    else if (AyBy > COLLISION_DISTANCE)
        return false;
    float dst(std::sqrt(AxBx*AxBx + AyBy*AyBy));
    return dst < COLLISION_DISTANCE;
}

void init_particles () {
    delete_particles();
    allParticles[0] = new Particle {0, 0, true};
    currentTotalParticles++;
    for (int i=0; i<MAX_SIMULTANEOUS; i++) {
        add_new_particle();
    }
}

void delete_particles() {
    for (int i=0; i<MAX_PARTICLE; i++) {
        delete allParticles[i];
        allParticles[i] = nullptr;
    }
}

void add_new_particle() {
    if (currentTotalParticles >= MAX_PARTICLE) {
        return;
    }
    Particle* P = new Particle {WIDTH, 0, false};
    allParticles[currentTotalParticles] = P;
    currentTotalParticles++;
}

void check_collisions() {
    // i for the free and j for the fixed
    for (int i=0; i<currentTotalParticles; i++) {
        if (allParticles[i]->isFixed == true)
            continue;
        for (int j=0; j<currentTotalParticles; j++) {
            if (allParticles[j]->isFixed == false)
                continue;
            if (is_collision(allParticles[i], allParticles[j])) {
                allParticles[i]->isFixed = true;
                add_new_particle();
                break;
            }
        }
    }
}

void update_particles() {
    for (int i=0; i<currentTotalParticles; i++) {
        if (allParticles[i]->isFixed)
            continue;
        allParticles[i]->update();
        allParticles[i]->border_control();
    }
}

void draw_particle(Particle* p) {
    glColor3f(1.0, 1.0, 1.0);
    //float dist = std::sqrt(p->x*p->x + p->y*p->y);
    
    for (int rot=0; rot<360; rot+=120) {
        glPushMatrix();
        glRotatef(rot, 0, 0, 1);
        glBegin(GL_POLYGON);
        for (float i=0; i<TWO_PI; i+=TWO_PI/10) {
            glVertex2f(p->x + std::cos(i)*DOT_RADIUS,
                       p->y + std::sin(i)*DOT_RADIUS);
        }
        glEnd();
        glPopMatrix();
    }
    for (int rot=0; rot<360; rot+=120) {
        glPushMatrix();
        glRotatef(rot, 0, 0, 1);
        glRotatef(180, 0, 1, 0);
        
        glBegin(GL_POLYGON);
        for (float i=0; i<TWO_PI; i+=TWO_PI/10) {
            glVertex2f(p->x + std::cos(i)*DOT_RADIUS,
                       p->y + std::sin(i)*DOT_RADIUS);
        }
        glEnd();
        glPopMatrix();
    }
    /*
    glColor3f(0.2, 0.2, 0.2);
    for (float x=PI/6; x<TWO_PI; x+=PI3) {
        glBegin(GL_LINE_STRIP);
          glVertex2f(0, 0);
          glVertex2f(WIDTH*std::cos(x), HEIGHT*std::sin(x));
        glEnd();
    }*/
}


void init() {
    glClearColor(0.1, 0.1, 0.1, 1.0);
    std::srand(std::time(0));
    init_particles();
}

int theta(0);
void display_callback() {
    glClear (GL_COLOR_BUFFER_BIT);
    //glRotatef(theta, 0.0, 0.0, 1.0);
    theta+=PI4/2; // in radians
    glColor3f(1.0, 1.0, 0.5);
    for (int i=0; i<currentTotalParticles; i++) {
        draw_particle(allParticles[i]);
    }

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
    auto start(std::chrono::steady_clock::now());
    if (currentTotalParticles >= MAX_PARTICLE) {
        currentTotalParticles = 0;
        init_particles();
    }
    //while(currentTotalParticles < MAX_PARTICLE) {
    for (int i=0; i<1000; i++) {
        check_collisions();
        update_particles();
    }
    glutPostRedisplay(); // run the display_callback function

    auto stop(std::chrono::steady_clock::now());
    auto duration(std::chrono::duration_cast<std::chrono::milliseconds>(stop-start));
    glutTimerFunc(std::abs(1000.0/FPS - duration.count()), timer_callback, 0);
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
