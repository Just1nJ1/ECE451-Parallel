#include <cuda.h>
#include <stdio.h>

__global__ void square(int* array, int n) {
    int tid = blockDim.x * blockIdx.x + threadIdx.x;
    printf("%d ", tid);
    if (tid < n) { // checks for thread index out of bounds
        array[tid] = array[tid] * array[tid];
    }
}

int main() {
    const int n = 1000;
    int* a = new int[n];
    for (int i = 0; i < n; i++) {
        a[i] = i;
    }

    int* dev_a;
    size_t size = n * sizeof(int);

    // Allocate memory on the device
    cudaError_t err = cudaMalloc(&dev_a, size);
    if (err != cudaSuccess) {
        printf("Error in cudaMalloc: %s\n", cudaGetErrorString(err));
        return 1;
    }

    // Copy data from host to device
    err = cudaMemcpy(dev_a, a, size, cudaMemcpyHostToDevice);
    if (err != cudaSuccess) {
        printf("Error in cudaMemcpy (HostToDevice): %s\n", cudaGetErrorString(err));
        cudaFree(dev_a);
        return 1;
    }

    // Determine the number of blocks and threads
    int threadsPerBlock = 256;  // arbitrarily picking 256 threads per block
    //int blocks = n / threadsPerBlock; // THIS IS WRONG: rounds down
    int blocks = (n + threadsPerBlock - 1) / threadsPerBlock; // round up to the min number of blocks needed

    printf("thread ids:");
    // Launch kernel with the correct number of blocks and threads
    square<<<blocks, threadsPerBlock>>>(dev_a, n);
    printf("\n======\n");

// is the kernel call syncronized automatically?
#if 0
    // Synchronize to make sure kernel execution is finished
    err = cudaDeviceSynchronize();
    if (err != cudaSuccess) {
        printf("Error in kernel execution: %s\n", cudaGetErrorString(err));
        cudaFree(dev_a);
        return 1;
    }
#endif
    // Copy the results back to host
    err = cudaMemcpy(a, dev_a, size, cudaMemcpyDeviceToHost);
    if (err != cudaSuccess) {
        printf("Error in cudaMemcpy (DeviceToHost): %s\n", cudaGetErrorString(err));
        cudaFree(dev_a);
        return 1;
    }

    printf("The squares are:\n");
    // Print the first 64 elements
    for (int i = 0; i < n; i++) {
        printf("%d ", a[i]);
    }
    printf("\n");

    // Clean up
    cudaFree(dev_a);
    delete[] a;

    return 0;
}
