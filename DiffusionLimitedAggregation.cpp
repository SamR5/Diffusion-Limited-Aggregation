
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
#define FPS 1

#define PI 3.1415926
#define TWO_PI 6.283185

#define MAX_PARTICLE 10000
#define MAX_SIMULTANEOUS 100
#define RADIUS 1
#define SPEED 2
#define OVERLAP_TOL 0

const float COLLISION_DISTANCE = RADIUS*2-OVERLAP_TOL;
// avoid the sqrt each for each distance calculus
const float COLLISION_DISTANCE2 = COLLISION_DISTANCE*COLLISION_DISTANCE;

struct Particle {
    int x, y;
    void go() {
        x += (rand()%2) ? 1 : -1;
        y += (rand()%2) ? 1 : -1;
    }
    void border_control() {
        if (x<-WIDTH/2)
            x=-WIDTH/2;
        else if (x>WIDTH/2)
            x=WIDTH/2;
        if (y<-HEIGHT/2)
            y=-HEIGHT/2;
        else if (y>HEIGHT/2)
            y=HEIGHT/2;
    }
};


bool is_collision(Particle& A, Particle& B);
void init_particles ();
void add_new_particle(int n);
void check_collisions();
void update_particles();
void check_out_of_bound();
void draw_dot(float x, float y);

Particle fixedParticles[MAX_PARTICLE];
Particle movingParticles[MAX_SIMULTANEOUS];

int currentTotalParticles(0); // current amount of particles in screen
int totalFixedParticles(0);

// distance of the farthest particle from the center
float farthest(10);


bool is_collision(Particle& A, Particle& B) {
    float X(A.x - B.x), Y(A.y - B.y);
    if (std::abs(X) > COLLISION_DISTANCE)
        return false;
    else if (std::abs(Y) > COLLISION_DISTANCE)
        return false;
    return X*X + Y*Y < COLLISION_DISTANCE2;
}

float distance_from_center(Particle& p) {
    return std::sqrt(p.x*p.x + p.y*p.y);
}

void init_particles () {
    fixedParticles[0] = Particle {0, 0};
    totalFixedParticles++;
    currentTotalParticles++;
    for (int i=0; i<MAX_SIMULTANEOUS; i++) {
        add_new_particle(i);
    }
}

void add_new_particle(int n) {
    if (totalFixedParticles >= MAX_PARTICLE)
        return;
    Particle P;
    
    int radius = farthest+10;
    float angle = rand()%360;
    angle *= PI / 180.0;
    
    P = {(int)(radius*std::cos(angle)), (int)(radius*std::sin(angle))};
    movingParticles[n] = P;
    currentTotalParticles++;
}

void check_collisions() {
    // the total can be slightly exceeded but this check is done
    // here for optimization purposes
    if (totalFixedParticles >= MAX_PARTICLE-1)
        return;
    float tempDist;
    // i for the free and j for the fixed
    for (int j=0; j<MAX_SIMULTANEOUS; j++) {
        tempDist = distance_from_center(movingParticles[j]);
        if (tempDist > farthest+RADIUS)
            continue;
        for (int i=totalFixedParticles-1; i>=0; i--) {
            if (is_collision(fixedParticles[i], movingParticles[j])) {
                fixedParticles[totalFixedParticles] = movingParticles[j];
                totalFixedParticles++;
                if (tempDist > farthest)
                    farthest = tempDist+RADIUS;
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

void check_out_of_bound() {
    for (int i=0; i<MAX_SIMULTANEOUS; i++) {
        if (distance_from_center(movingParticles[i]) > farthest+20)
            add_new_particle(i);
    }
}

void draw_dot(float x, float y) {
    glBegin(GL_POLYGON);
    for (float i=0; i<TWO_PI; i+=TWO_PI/10) {
        glVertex2f(x + std::cos(i)*RADIUS, y + std::sin(i)*RADIUS);
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
        glVertex2f(fixedParticles[i].x, fixedParticles[i].y);
    }
    /*for (int i=0; i<MAX_SIMULTANEOUS; i++) {
        glVertex2f(movingParticles[i].x, movingParticles[i].y);
    }*/
    

    glEnd();
    glFlush();
    glutSwapBuffers();
}

void reshape_callback(int width, int height) {
    glViewport(0, 0, (GLsizei)width, (GLsizei) height);
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    glOrtho(-WIDTH/2, WIDTH/2,
            -HEIGHT/2, HEIGHT/2,
            -1.0, 0.0);
    glMatrixMode(GL_MODELVIEW);
}

void timer_callback(int) {
    if (totalFixedParticles >= MAX_PARTICLE-1) {
        std::cout << "Finished\n";
        return;
    }
    auto start(std::chrono::steady_clock::now());
    
    for (int i=0; i<10000; i++) {
        check_collisions();
        update_particles();
    }
    check_out_of_bound();
    
    glutPostRedisplay(); // run the display_callback function
    
    auto stop(std::chrono::steady_clock::now());
    auto duration(std::chrono::duration_cast<std::chrono::milliseconds>(stop-start));
    std::cout << duration.count() << "ms\n";
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
