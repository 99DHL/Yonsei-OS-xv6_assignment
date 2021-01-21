#ifndef __MANDELBROT_H__
#define __MANDELBROT_H__

#define max_iterations 2048     // Max number of iterations
#define resolution     768      // Mandelbrot window resolution
#define escape         4.0      // Mandelbrot escape value
#define minW          -2.2      // Min W
#define maxW           1.0      // Max W
#define minH          -1.5      // Min H
#define maxH           1.6      // Max H

// Number of threads
extern unsigned num_threads;
// Mandelbrot convergence map
extern unsigned mandelbrot[resolution*resolution];

// Calculate the Mandelbrot convergence map.
void calc_mandelbrot();

// Complex number
class Complex {
public:
    Complex(float a, float b) : r(a), i(b) {}
    float magnitude2() { return (r*r + i*i); }
    Complex operator*(const Complex &c) { return Complex(r*c.r - i*c.i, i*c.r + r*c.i); }
    Complex operator+(const Complex &c) { return Complex(r+c.r, i+c.i); }
private:
    float r, i;
};

#endif

