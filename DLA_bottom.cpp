
/* Diffusion Limited Aggregation */

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
#define FPS 10

#define PI 3.1415926
#define TWO_PI 6.283185

#define MAX_PARTICLE 50000
#define MAX_SIMULTANEOUS 50
#define DOT_RADIUS 1
#define OVERLAP_TOL 0


const float COLLISION_DISTANCE = DOT_RADIUS*2-OVERLAP_TOL;
// avoid the sqrt each for each distance calculus
const float COLLISION_DISTANCE2 = COLLISION_DISTANCE*COLLISION_DISTANCE;

struct Particle {
    float x, y;
    void go() {
        x += (rand()%2) ? 1 : -1;
        y += (rand()%3) ? -1 : 1;
    }
    void border_control() {
        if (x<0)
            x=0;
        else if (x>WIDTH)
            x=WIDTH;
    }
    bool touch_limit() {
        return y <= DOT_RADIUS;
    }
};


bool is_collision(Particle& A, Particle& B);
void init_particles ();
void add_new_particle(int n);
void check_collisions();
void update_particles();

Particle fixedParticles[MAX_PARTICLE];
Particle movingParticles[MAX_SIMULTANEOUS];

int currentTotalParticles(0); // current amount of particles in screen
int totalFixedParticles(0);

// keeps track of the highest particle and only check
// collisions below
float highest(DOT_RADIUS*2);

bool is_collision(Particle& A, Particle& B) {
    float X(A.x - B.x), Y(A.y - B.y);
    if (std::abs(X) > COLLISION_DISTANCE)
        return false;
    else if (std::abs(Y) > COLLISION_DISTANCE)
        return false;
    return X*X + Y*Y < COLLISION_DISTANCE2;
}

void init_particles () {
    for (int i=0; i<MAX_SIMULTANEOUS; i++) {
        add_new_particle(i);
    }
}

void add_new_particle(int n) {
    if (currentTotalParticles >= MAX_PARTICLE)
        return;
    Particle P = {rand()%WIDTH, HEIGHT};
    movingParticles[n] = P;
    currentTotalParticles++;
}

void check_collisions() {
    // the total can be slightly exceeded but this check is done
    // here for optimization purposes
    if (totalFixedParticles >= MAX_PARTICLE-1)
        return;
    // i for the free and j for the fixed
    for (int j=0; j<MAX_SIMULTANEOUS; j++) {
        if (movingParticles[j].y > highest+DOT_RADIUS*2)
            continue;
        for (int i=totalFixedParticles-1; i>=0; i--) {
            if (is_collision(fixedParticles[i], movingParticles[j])) {
                if (movingParticles[j].y > highest)
                    highest = movingParticles[j].y+DOT_RADIUS*2;
                fixedParticles[totalFixedParticles] = movingParticles[j];
                totalFixedParticles++;
                add_new_particle(j);
                break;
            }
        }
    }
    for (int j=0; j<MAX_SIMULTANEOUS; j++) {
        if (movingParticles[j].touch_limit()) {
            fixedParticles[totalFixedParticles] = movingParticles[j];
            totalFixedParticles++;
            add_new_particle(j);
            break;
        }
    }
}

void update_particles() {
    for (int i=0; i<MAX_SIMULTANEOUS; i++) {
        movingParticles[i].go();
        movingParticles[i].border_control();
    }
}

void init() {
    glClearColor(0.1, 0.1, 0.1, 1.0);
    std::srand(std::time(0));
    init_particles();
}

void display_callback() {
    glClear(GL_COLOR_BUFFER_BIT);

    glBegin(GL_POINTS);
    glColor3f(1.0, 1.0, 1.0);
    for (int i=0; i<totalFixedParticles; i++) {
        /*glBegin(GL_LINE_LOOP);
        for (float k=0; k<TWO_PI; k+=TWO_PI/100) {
            glVertex2f(fixedParticles[i].x + DOT_RADIUS*std::cos(k),
                       fixedParticles[i].y + DOT_RADIUS*std::sin(k));
        }
        glEnd();*/
        glVertex2f(fixedParticles[i].x, fixedParticles[i].y);
    }
    glEnd();
    /*glBegin(GL_POINTS);
    for (int i=0; i<MAX_SIMULTANEOUS; i++) {
        //glRectf(fixedParticles[i].x, fixedParticles[i].y,
        //        fixedParticles[i].x+1, fixedParticles[i].y+1);
        glVertex2f(movingParticles[i].x, movingParticles[i].y);
    }
    glEnd();*/
    glFlush();
    glutSwapBuffers();
}

void reshape_callback(int width, int height) {
    glViewport(0, 0, (GLsizei)width, (GLsizei) height);
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    int margin(0);
    glOrtho(0.0-margin, (double)WIDTH+margin,
            0.0-margin, (double)HEIGHT+margin,
            -1.0, 0.0);
    glMatrixMode(GL_MODELVIEW);
}

void timer_callback(int) {
    if (totalFixedParticles >= MAX_PARTICLE-1) {
        std::cout << "Finished\n";
        return;
    }
    auto start(std::chrono::steady_clock::now());
    
    int newP(totalFixedParticles);
    for (int i=0; i<10000; i++) {
        check_collisions();
        update_particles();
    }
    glutPostRedisplay(); // run the display_callback function
    
    auto stop(std::chrono::steady_clock::now());
    auto duration(std::chrono::duration_cast<std::chrono::milliseconds>(stop-start));
    //std::cout << duration.count() << "ms\n";
    glutTimerFunc(1000/FPS, timer_callback, 0);
}


int main(int argc, char **argv) {
    glutInit(&argc, argv); // initialize
    glutInitDisplayMode(GLUT_RGB | GLUT_DOUBLE);
    glutInitWindowPosition(15, 15); // optional
    glutInitWindowSize(WIDTH, HEIGHT);
    glutCreateWindow("Diffusion-Limited Aggregation");
    glutDisplayFunc(display_callback);
    glutReshapeFunc(reshape_callback);
    glutTimerFunc(1000/FPS, timer_callback, 0);
    init();
    glutMainLoop();

    return 0;
}
