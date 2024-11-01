#include <iostream>
#include <chrono>
using namespace std;

//cannot be read from cache the first time
// cannot fit into cache, therefore MISS
// by the time you get back to the beginning, cache is remembering the end

// while it does not work with cache
// sequential access is the fastest way RAM works
double a(volatile double x[], int n) {
    double sum = 0;
    for (int i = 0; i < n; i++)
        sum += x[i];
    return sum;
}

// go backwards through memory (just as fast as forwards)
double b(volatile double x[], int n) {
	double sum = 0;
	for (int i = n-1; i >= 0; i--)
    sum += x[i];
	return sum;
}

// this is faster (trick question) because it's doing 1/16 the work
double c(volatile double x[], int n) {
    double sum = 0;
    for (int i = 0; i < n; i+=16)
        sum += x[i];
    return sum;
}

double d(volatile double x[], int n) {
    double sum = 0;
    for (int i = 0; i < n; i+=16)
        sum += x[i];
    return sum;
}

// increment every element by 1
// forget about cache, we are writing!
void e(volatile double x[], int n) {
    for (int i = 0; i < n; i++)
       x[i]++;
}

void f(volatile double x[], int stride, int n) {
    for (int i = 0; i < stride; i++) {
        for (int j = i; j < n; i+=stride)
            x[j]++;
    }
}

double g(volatile double x[], int stride,int n) {
    double sum = 0;
    for (int i = 0; i < stride; i++) {
        for (int j = i; j < n; j+=stride)
            sum += x[j];
    }
    return sum;
}

double h(volatile double x[], int stride,int n) {
    double sum = 0;
    for (int i = 0; i < n; i++) {
        sum += x[0];
    }
    return sum; // x[0] * n
}

// read from elements [0] to [1023] repeatedly
double j(volatile double x[], int stride,int n) {
    const int size = 1024;
    double sum = 0;
    for (int i = 0; i < n; i+=size) {
        for (int j = 0; j < size; j++)
            sum += x[j];
    }
    return sum;
}

// first pass no cache (you never read it)
// subsequent pass cached for read, but write
void k(volatile double x[], int stride,int n) {
    const int size = 1024;
    for (int i = 0; i < n; i+=size) {
        for (int j = 0; j < size; j++)
            x[j]++;
    }
}

int main() {
    const int n = 100'000'000;
    const int num_trials = 100;
    double* x = new double[n];

    double sum = 0;
    auto start = chrono::high_resolution_clock::now();
    for (int trial = 0; trial < num_trials; trial++) {
        sum  = a(x, n);
    }
    auto end = chrono::high_resolution_clock::now();
    cout << chrono::duration_cast<chrono::microseconds>(end - start).count()*1e-6/num_trials << "sum = " << sum << endl;

    start = chrono::high_resolution_clock::now();
    for (int trial = 0; trial < num_trials; trial++) {
        sum  = b(x, n);
    }
    end = chrono::high_resolution_clock::now();
    cout << chrono::duration_cast<chrono::microseconds>(end - start).count()*1e-6/num_trials << "sum = " << sum << endl;

    start = chrono::high_resolution_clock::now();
    for (int trial = 0; trial < num_trials; trial++) {
        sum  = c(x, n);
    }
    end = chrono::high_resolution_clock::now();
    cout << chrono::duration_cast<chrono::microseconds>(end - start).count()*1e-6/num_trials << "sum = " << sum << endl;

    start = chrono::high_resolution_clock::now();
    for (int trial = 0; trial < 10; trial++) {
        sum  = d(x, n);
    }
    end = chrono::high_resolution_clock::now();
    cout << chrono::duration_cast<chrono::microseconds>(end - start).count()*1e-6/num_trials << "sum = " << sum << endl;

    start = chrono::high_resolution_clock::now();
    for (int trial = 0; trial < num_trials; trial++) {
        e(x, n);
    }
    end = chrono::high_resolution_clock::now();
    cout << chrono::duration_cast<chrono::microseconds>(end - start).count()*1e-6/num_trials << "sum = " << sum << endl;

    for (int stride = 1; stride <= 1024; stride*=2) {
        start = chrono::high_resolution_clock::now();
        for (int trial = 0; trial < num_trials; trial++) {
            f(x, stride, n);
        }
        end = chrono::high_resolution_clock::now();
        cout << chrono::duration_cast<chrono::microseconds>(end - start).count()*1e-6/num_trials << "sum = " << sum << endl;
    }
    start = chrono::high_resolution_clock::now();
    for (int trial = 0; trial < num_trials; trial++) {
        sum  = g(x, 16, n);
    }
    end = chrono::high_resolution_clock::now();
    cout << chrono::duration_cast<chrono::microseconds>(end - start).count()*1e-6/num_trials << "sum = " << sum << endl;

    start = chrono::high_resolution_clock::now();
    for (int trial = 0; trial < num_trials; trial++) {
        sum  = h(x, 16, n);
    }
    end = chrono::high_resolution_clock::now();
    cout << chrono::duration_cast<chrono::microseconds>(end - start).count()*1e-6/num_trials << "sum = " << sum << endl;

    start = chrono::high_resolution_clock::now();
    for (int trial = 0; trial < 10; trial++) {
        sum  = j(x, 16, n);
    }
    end = chrono::high_resolution_clock::now();
    cout << chrono::duration_cast<chrono::microseconds>(end - start).count()*1e-6/num_trials << "sum = " << sum << endl;

    start = chrono::high_resolution_clock::now();
    for (int trial = 0; trial < num_trials; trial++) {
        k(x, 16, n);
    }
    end = chrono::high_resolution_clock::now();
    cout << chrono::duration_cast<chrono::microseconds>(end - start).count()*1e-6/10 << "sum = " << sum << endl;
    
    return 0;
}
