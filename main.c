#include "hash.h"

int main(int argc, char *argv[])
{
  /* input: number of points to test, number of cells per axis */
  const int nbins = 20;
  double Lbox;
  int input_ngrid;
  float input_boxsize;
  char filename[1000];

  /* check inputs */
  if(argc != 4) {
    printf("./auto ngrid box_size filename\n");
    exit(-1);
  }

  input_ngrid = atoi(argv[1]);
  input_boxsize = atof(argv[2]);

  sprintf(filename,"%s",argv[3]);

  if(input_ngrid <= 0) {
    printf("ngrid must be positive!\n");
    exit(-1);
  }
  if(input_boxsize <= 0) {
    printf("boxsize must be positive!\n");
    exit(-1);
  }

  size_t npoints;
  int ngrid;
  ngrid = (int)input_ngrid;
  Lbox = (double)input_boxsize;

  /* read from file */
  particle *points = read_particles_hdf5(filename, "particles", &npoints);

  // separate arrays (or Fortran-style arrays) are necessary both for SIMD and cache efficiency
  FLOAT *x = (FLOAT*) my_malloc(npoints*sizeof(FLOAT));
  FLOAT *y = (FLOAT*) my_malloc(npoints*sizeof(FLOAT));
  FLOAT *z = (FLOAT*) my_malloc(npoints*sizeof(FLOAT));

  size_t n;
  for(n=0;n<npoints;n++)
    {
      x[n] = points[n].x;
      y[n] = points[n].y;
      z[n] = points[n].z;
    }

  free(points);

  /* hash into grid cells */
  GHash *grid = allocate_hash(ngrid, Lbox, npoints, x, y, z);
  if ((int)grid == 0) {
    printf("allocating grid failed!\n");
    exit(-1);
  }

  geometric_hash(grid, x, y, z, npoints);

  /* compute pair counts assuming periodic box */
  double *bin_edges_sq = my_malloc((nbins+1)*sizeof(double));
  double *bin_edges = my_malloc((nbins+1)*sizeof(double));
  long int *pcounts = my_malloc(nbins*sizeof(long int));
  long int *pcounts_naive = my_malloc(nbins*sizeof(long int));
  int i;
  for(i=0;i<nbins;i++) {
    pcounts[i] = (long int) 0;
    pcounts_naive[i] = (long int) 0;
  }
  double maxr = grid->Lbox/(double)(grid->ngrid);
  double minr = 0.1;
  double dlogr = (log10(maxr)-log10(minr))/(double)nbins;
  for(i=0;i<=nbins;i++) {
    double bin_edge = pow(10.0, ((double)i)*dlogr + log10(minr));
    bin_edges[i] = bin_edge;
    bin_edges_sq[i] = SQ(bin_edge);
  }

  count_pairs(grid, pcounts, bin_edges_sq, nbins);
#ifdef TEST_ALL_PAIRS
  count_pairs_naive(x,y,z, npoints, pcounts_naive, bin_edges_sq, nbins, Lbox);
#endif

  /* output pair counts */
  for(i=0;i<nbins;i++) {
    double ndens = npoints/CUBE(Lbox);
    double exp_counts = (2./3.)*M_PI*(CUBE(bin_edges[i+1])-CUBE(bin_edges[i]))*ndens*npoints;
    printf("pair counts between (%lf, %lf] = %ld\n",bin_edges[i],bin_edges[i+1],pcounts[i]);
#ifdef TEST_ALL_PAIRS
    printf("(naive) pair counts = %ld\n",pcounts_naive[i]);
#endif
    printf("expected pair counts = %lf\n",exp_counts);
    printf("correlation function = %lf\n\n",(double)pcounts[i]/exp_counts);
  }

  /* free memory */
  my_free(pcounts);
  my_free(bin_edges);
  my_free(bin_edges_sq);

  my_free(x);
  my_free(y);
  my_free(z);
  
  free_hash(grid);
  return 0;
}
