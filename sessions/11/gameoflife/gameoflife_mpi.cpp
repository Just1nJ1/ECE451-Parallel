#include <mpi.h>
#include <iostream>
#include <cstdint>
using namespace std;


int num_cpus;
int id;
const uint32_t cpu_grid = 4;
const uint32_t n = 10;
const uint32_t row = n + 2;
const uint32_t grid_size = 4;
uint8_t* life;
uint8_t* nextlife;
uint8_t* westbuf;
uint8_t* eastbuf;


void init() {
    const uint32_t size = (n+2)*(n+2);
    life = new uint8_t[size];
    nextlife = new uint8_t[size];

    for (int i = 0; i < size; i++)
      life[i] = 0;

    //east-west buffers need n + 2 elements to include the incoming value from NORTH/SOUTH
    westbuf = new uint8_t[row];
    eastbuf = new uint8_t[row];
}

/*
    vector approach

    10010001


       1    0    0    1    0    0    0    1
    0000 0000 0000 0000 0000 0000 0000 0000

    00000000000000000000000000000000
    00000000000000000000110000000000
    00000000000000000000110000000000
    00000000000000000000000000000000

    CPU

    0   1   2   3
    4   5   6   7
    8   9  10  11
   12  13  14  15



    0   1
    2   3

    n  = 8192

    0                                      1

    0 0 0 0 0 0 0 0 0 0 0                  0 0 0 0 0 0 0 0 0 0 0
    0 0 0 0 0 0 0 0 0 0 0                  0 0 0 0 0 0 0 0 0 0 0
    0 0 0 0 0 0 0 0 0 1 1                  0 0 0 0 0 0 0 0 0 0 0
    0 0 0 0 0 0 0 0 0 0 0                  0 0 0 0 0 0 0 0 0 0 0
    0 0 0 0 0 0 0 0 0 0 0                  0 0 0 0 0 0 0 0 0 0 0
    0 0 0 0 0 0 0 0 0 0 0                  0 0 0 0 0 0 0 0 0 0 0
    0 0 0 0 0 0 0 0 0 0 0                  0 0 0 0 0 0 0 0 0 0 0
    0 0 0 0 0 0 0 0 0 0 0                  0 0 0 0 0 0 0 0 0 0 0
    0 0 0 0 0 0 0 0 0 0 0                  0 0 0 0 0 0 0 0 0 0 0
    0 0 0 0 0 0 0 0 0 0 0                  0 0 0 0 0 0 0 0 0 0 0

each grid is surrounded by a buffer of zeros.
Now we will use it by copying in the value from the neighbor
// TODO: DIAGONALS STINK! We have to do

  x x x x x x x x x x x x
  x 1 1 1 0 0 0 0 0 0 0 x                 0 0 0 0 0 0 0 0 0 0 0
  x 0 0 0 0 0 0 0 0 0 0 x                 0 0 0 0 0 0 0 0 0 0 0
  x 0 0 0 0 0 0 0 0 0 0 x                 0 0 0 0 0 0 0 0 0 0 0
  x 0 0 0 0 0 0 0 0 0 0 x                 0 0 0 0 0 0 0 0 0 0 0
  x 0 0 0 0 0 0 0 0 0 0 x                 0 0 0 0 0 0 0 0 0 0 0
  x 0 0 0 0 0 0 0 0 0 0 x                 0 0 0 0 0 0 0 0 0 0 0
  x 0 0 0 0 0 0 0 0 0 0 x                 0 0 0 0 0 0 0 0 0 0 0
  x 0 0 0 0 0 0 0 0 0 0 x                 0 0 0 0 0 0 0 0 0 0 0
  x 0 0 0 0 0 0 0 0 0 0 x                 0 0 0 0 0 0 0 0 0 0 0
  x 0 0 0 0 0 0 0 0 0 0 x                 0 0 0 0 0 0 0 0 0 0 0
  x x x x x x x x x x x x 
    1                                        2
*/

void calcLiveOrDead(uint32_t i) {
    const uint32_t EAST = +1;
    const uint32_t WEST = -1;
    const uint32_t NORTH = -n-2;
    const uint32_t SOUTH = +n+2;
  int count = life[i+EAST] + life[i+WEST] + life[i+NORTH] + life[i+SOUTH] +
        life[i+NORTH+EAST] + life[i+NORTH + WEST] + life[i+SOUTH + EAST] + life[i+SOUTH+WEST];
  if (life[i]) {
    nextlife[i] = count >=2 && count <= 3;
  } else {
    nextlife[i] = count == 3;

  }
 }

void stepForward() {
  // first copy to/from our neighbors
  if (id >= grid_size) {
     MPI_Send(&life[row+1], n, MPI_CHAR, id-grid_size, 1, MPI_COMM_WORLD);
  }

  if (id < num_cpus - grid_size) {// highest with neightbor to south = 11
     MPI_Send(&life[row*n+1], n, MPI_CHAR, id+grid_size, 1, MPI_COMM_WORLD);     
     MPI_Recv(&life[1], n, MPI_CHAR, id+grid_size, 1, MPI_COMM_WORLD, MPI_STATUS_IGNORE);    
  }

  if (id >= grid_size) {
     MPI_Recv(&life[row*n+1], n, MPI_CHAR, id-grid_size, 1, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
  }

  if (id % grid_size > 0) {
    for (int i = 0, c = 1; i < row; i++, c += row)
      westbuf[i] = life[c]; // copy into a sequential buffer
    MPI_Send(westbuf, row, MPI_CHAR, id-1, 1, MPI_COMM_WORLD);
  }

  if (id % grid_size  < grid_size-1) {
    for (int i = 0, c = n; i < row; i++, c += row)
      eastbuf[i] = life[c]; // copy our eastern edge to send east
    MPI_Send(eastbuf, row, MPI_CHAR, id+1, 1, MPI_COMM_WORLD);

    MPI_Recv(eastbuf, row, MPI_CHAR, id+1, 1, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
    for (int i = 0, c = n+1; i < row; i++, c += row)
      life[c] = eastbuf[i]; // copy fromsequential buffer into our array
  }

  if (id % grid_size > 0) {
    MPI_Recv(westbuf, row, MPI_CHAR, id-1, 1, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
  }

  for (int i = 0, c = n+2+1; i < n; i++, c+= 2)
    for (int j = 0; j < n ; j++, c++)
      calcLiveOrDead(c);
  swap(life, nextlife);
}

void print() {
    for (int i = 0, c = n+2+1; i < n; i++, c += 2) {
      for (int j = 0; j < n; j++, c++)
        cout << int(life[c]) << ' ';
      cout << "\n";
    }
    cout << "\n\n" << endl;
}

int main(int argc, char* argv[]) {
  MPI_Init(&argc, &argv);
  MPI_Comm_size(MPI_COMM_WORLD, &num_cpus);
  MPI_Comm_rank(MPI_COMM_WORLD, &id);
  cout << "I am cpu: " << id << " numcpus=" << num_cpus << '\n';
  int num_generations = 25;
  init();
  if (id == 0) {
    //create a glider
    life[1*row+4] = 1;
    life[2*row+5] = 1;
    life[3*row+3] = 1;
    life[3*row+4] = 1;
    life[3*row+5] = 1;
  }
  for (int i = 0; i < num_generations; i++) {

    stepForward();
    if (id == 0) {
      cout << "gen: " << i << endl;
      print();
    }
  }
  MPI_Finalize();
}
