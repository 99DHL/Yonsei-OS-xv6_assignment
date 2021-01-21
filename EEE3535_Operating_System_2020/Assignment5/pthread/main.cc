#ifdef __APPLE__
#include <GLUT/glut.h>
#else
#include <GL/glut.h>
#endif
#include <iostream>
#include <cstring>
#include "mandelbrot.h"
#include "stopwatch.h"

// Number of threads
unsigned num_threads = 1;
// Mandelbrot color palette
float colormap[32][3];
// OpenGL window ID
int window_id = 0;

void set_color(float *c, unsigned r, unsigned g, unsigned b) {
    c[0] = (float)r/255; c[1] = (float)g/255; c[2] = (float)b/255;
}

// Initialize the Mandelbrot color palette.
void init_colormap(void) {
    set_color(colormap[ 0],   0,   0,   0); set_color(colormap[ 1],   0,   0,   8);
    set_color(colormap[ 2],   0,   0,  16); set_color(colormap[ 3],   4,   0,  31);
    set_color(colormap[ 4],   9,   1,  47); set_color(colormap[ 5],   6,   2,  60);
    set_color(colormap[ 6],   4,   4,  73); set_color(colormap[ 7],   2,   5,  86);
    set_color(colormap[ 8],   0,   7, 100); set_color(colormap[ 9],   6,  25, 119);
    set_color(colormap[10],  12,  44, 138); set_color(colormap[11],  18,  63, 157);
    set_color(colormap[12],  24,  82, 177); set_color(colormap[13],  40, 103, 193);
    set_color(colormap[14],  57, 125, 209); set_color(colormap[15],  95, 153, 219);
    set_color(colormap[16], 134, 181, 229); set_color(colormap[17], 172, 208, 238);
    set_color(colormap[18], 211, 236, 248); set_color(colormap[19], 226, 234, 219);
    set_color(colormap[20], 241, 233, 191); set_color(colormap[21], 244, 217, 143);
    set_color(colormap[22], 248, 201,  95); set_color(colormap[23], 251, 185,  47);
    set_color(colormap[24], 255, 170,   0); set_color(colormap[25], 255, 170,   0);
    set_color(colormap[26], 204, 128,   0); set_color(colormap[27], 178, 107,   0);
    set_color(colormap[28], 153,  87,   0); set_color(colormap[29], 130,  70,   1);
    set_color(colormap[30], 106,  52,   3); set_color(colormap[31],  82,  34,   5);

    memset(mandelbrot, 0, resolution*resolution*sizeof(unsigned));
}

// Display an OpenGL window.
void display(void) {
#ifdef RETINA
    glViewport(0, 0, 2*resolution, 2*resolution);
#endif
    glClear(GL_COLOR_BUFFER_BIT);
    glBegin(GL_QUADS);
    for(unsigned w = 0; w < resolution; w++) {
        for(unsigned h = 0; h < resolution; h++) {
            glColor3fv(colormap[mandelbrot[w + h*resolution] & 0x1f]);
            glVertex2f(float(w)  *2/resolution-1, float(h)  *2/resolution-1);
            glVertex2f(float(w)  *2/resolution-1, float(h+1)*2/resolution-1);
            glVertex2f(float(w+1)*2/resolution-1, float(h+1)*2/resolution-1);
            glVertex2f(float(w+1)*2/resolution-1, float(h)  *2/resolution-1);
        }
    }
    glEnd();
    glFlush();
};

// Keyboard input for OpenGL window
void keyboard(unsigned char key, int x, int y) {
    if(key == 'q' || key == 'Q') { glutDestroyWindow(window_id); exit(0); }
}

int main(int argc, char **argv) {
    // Get the number of threads from the command run.
    if(argc != 2) {
        std::cerr << "Usage: " << argv[0] << " [num_threads]" << std::endl;
        exit(1);
    }
    num_threads = atoi(argv[1]);

    // Initialize the Mandelbrot color palette.
    init_colormap();
    // Measure the calculation time.
    stopwatch_t stopwatch;
    stopwatch.reset();
    stopwatch.start();
    // Calculate the Mandelbrot convergence map.
    calc_mandelbrot();
    stopwatch.stop();
    stopwatch.display();

    // OpenGL routine
    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_SINGLE | GLUT_RGB);
    glutInitWindowPosition(32, 32);
    glutInitWindowSize(resolution, resolution);
    window_id = glutCreateWindow("EEE3535 Operating Systems: Pthread Mandelbrot");
    glutDisplayFunc(&display); 
    glutKeyboardFunc(&keyboard);
    glutMainLoop();
    
    return 0;
}

