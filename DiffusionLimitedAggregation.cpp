
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

#define MAX_PARTICLE 10000
#define MAX_SIMULTANEOUS 50
#define DOT_RADIUS 1
#define SPEED 2
#define OVERLAP_TOL 0

const float COLLISION_DISTANCE = DOT_RADIUS*2-OVERLAP_TOL;
// avoid the sqrt each for each distance calculus
const float COLLISION_DISTANCE2 = COLLISION_DISTANCE*COLLISION_DISTANCE;

struct Particle {
    float x, y;
    void go() {
        //int xV(rand()%SPEED), yV(rand()%SPEED);
        //x += (rand()%2) ? xV : -xV;
        //y += (rand()%2) ? yV : -yV;
        x += (rand()%2) ? 1 : -1;
        y += (rand()%2) ? 1 : -1;
    }
    void border_control() {
        if (x<0)
            x=0;
        else if (x>WIDTH)
            x=WIDTH;
        if (y<0)
            y=0;
        else if (y>HEIGHT)
            y=HEIGHT;
    }
};


bool is_collision(Particle& A, Particle& B);
void init_particles ();
void add_new_particle(int n);
void check_collisions();
void update_particles();
void draw_dot(float x, float y);

Particle fixedParticles[MAX_PARTICLE];
Particle movingParticles[MAX_SIMULTANEOUS];

int currentTotalParticles(0); // current amount of particles in screen
int totalFixedParticles(0);

bool is_collision(Particle& A, Particle& B) {
    float X(A.x - B.x), Y(A.y - B.y);
    if (std::abs(X) > COLLISION_DISTANCE)
        return false;
    else if (std::abs(Y) > COLLISION_DISTANCE)
        return false;
    /*if (X > COLLISION_DISTANCE || -X > COLLISION_DISTANCE)
        return false;
    if (Y > COLLISION_DISTANCE || -Y > COLLISION_DISTANCE)
        return false;*/
    return X*X + Y*Y < COLLISION_DISTANCE2;
}

void init_particles () {
    fixedParticles[0] = Particle {WIDTH/2, HEIGHT/2};
    totalFixedParticles++;
    currentTotalParticles++;
    for (int i=0; i<MAX_SIMULTANEOUS; i++) {
        add_new_particle(i);
    }
}

void add_new_particle(int n) {
    if (currentTotalParticles >= MAX_PARTICLE)
        return;
    Particle P;
    // vertical sides or horizontal sides
    if (rand()%2) {
        P = {(rand()%2) ? 0 : WIDTH, rand()%HEIGHT};
    }
    else {
        P = {rand()%WIDTH, (rand()%2) ? 0 : HEIGHT};
    }
    movingParticles[n] = P;
    currentTotalParticles++;
}

void check_collisions() {
    // the total can be slightly exceeded but this check is done
    // here for optimization purposes
    if (totalFixedParticles >= MAX_PARTICLE-1)
        return;
    // i for the free and j for the fixed
    for (int i=totalFixedParticles-1; i>=0; i--) {
        for (int j=0; j<MAX_SIMULTANEOUS; j++) {
            if (is_collision(fixedParticles[i], movingParticles[j])) {
                fixedParticles[totalFixedParticles] = movingParticles[j];
                totalFixedParticles++;
                add_new_particle(j);
                break;
            }
        }
    }
}

void update_particles() {
    for (int i=0; i<MAX_SIMULTANEOUS; i++) {
        movingParticles[i].go();
        movingParticles[i].border_control();
    }
}

void draw_dot(float x, float y) {
    glBegin(GL_POLYGON);
    for (float i=0; i<TWO_PI; i+=TWO_PI/10) {
        glVertex2f(x + std::cos(i)*DOT_RADIUS, y + std::sin(i)*DOT_RADIUS);
    }
    glEnd();
}

void init() {
    glClearColor(0.1, 0.1, 0.1, 1.0);
    std::srand(std::time(0));
    init_particles();
}

void display_callback() {
    glClear (GL_COLOR_BUFFER_BIT);

    glColor3f(1.0, 1.0, 1.0);
    glPointSize(1);
    glBegin(GL_POINTS);
    for (int i=0; i<totalFixedParticles; i++) {
        //draw_dot(fixedParticles[i].x, fixedParticles[i].y);
        //glRectf(fixedParticles[i].x, fixedParticles[i].y,
        //        fixedParticles[i].x+1, fixedParticles[i].y+1);
        glVertex2f(fixedParticles[i].x, fixedParticles[i].y);
    }
    glEnd();
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
    for (int i=0; i<100000; i++) {
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
