#include "hash.h"

int main(int argc, char *argv[])
{
  /* input: number of points to test, number of cells per axis */
  const long seed = 42;
  const int nbins = 20;
  double Lbox = 1.0;
  int input_npoints, input_ngrid;
  float input_boxsize;

  /* check inputs */
  if(argc != 4) {
    printf("./hash ngrid npoints_on_side box_size\n");
    exit(-1);
  }

  input_ngrid = atoi(argv[1]);
  input_npoints = atoi(argv[2]);
  input_boxsize = atof(argv[3]);

  if(input_ngrid <= 0) {
    printf("ngrid must be positive!\n");
    exit(-1);
  }
  if(input_npoints <= 0) {
    printf("npoints must be positive\n");
    exit(-1);
  }
  if(input_boxsize <= 0) {
    printf("boxsize must be positive!\n");
    exit(-1);
  }

  size_t npoints;
  int ngrid;
  npoints = CUBE((size_t)input_npoints);
  ngrid = (int)input_ngrid;
  Lbox = (double)input_boxsize;

  printf("computing with %ld random points in a (%lf)^3 periodic box...\n",npoints,Lbox);

  /* generate random points (x,y,z) in unit cube */
  // separate arrays (or Fortran-style arrays) are necessary both for SIMD and cache efficiency
  FLOAT *x = (FLOAT*) my_malloc(npoints*sizeof(FLOAT));
  FLOAT *y = (FLOAT*) my_malloc(npoints*sizeof(FLOAT));
  FLOAT *z = (FLOAT*) my_malloc(npoints*sizeof(FLOAT));
  
  const gsl_rng_type * T;
  gsl_rng * r;
  gsl_rng_env_setup();
  T = gsl_rng_default;
  r = gsl_rng_alloc(T);
  gsl_rng_set(r, seed); /* Seeding random distribution */
  
  size_t n;
  for(n=0;n<npoints;n++)
    {
      x[n] = gsl_rng_uniform(r)*Lbox; 
      y[n] = gsl_rng_uniform(r)*Lbox; 
      z[n] = gsl_rng_uniform(r)*Lbox; 
    }

  gsl_rng_free(r);

  /* hash into grid cells */
  GHash *grid = allocate_hash(ngrid, Lbox, npoints);
  if ((int)grid == 0) {
    printf("allocating grid failed!\n");
    exit(-1);
  }

  geometric_hash(grid, x, y, z, npoints);

  //  free(x); free(y); free(z);
  
  /* output array of counts in cells */
  int i,j,k;
  for(i=0;i<ngrid;i++) {
    for(j=0;j<ngrid;j++) {
      for(k=0;k<ngrid;k++) {
	//	printf("[%ld][%ld][%ld] = %ld\n",i,j,k,grid->counts[INDEX(i,j,k)]);
      }
    }
  }

  /* compute pair counts */
  double *bin_edges_sq = my_malloc((nbins+1)*sizeof(double));
  double *bin_edges = my_malloc((nbins+1)*sizeof(double));
  long int *pcounts = my_malloc(nbins*sizeof(long int));
  long int *pcounts_naive = my_malloc(nbins*sizeof(long int));
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
  if(testing)
    count_pairs_naive(x,y,z, npoints, pcounts_naive, bin_edges_sq, nbins, Lbox);

  for(i=0;i<nbins;i++) {
    double ndens = npoints/CUBE(Lbox);
    double exp_counts = (2./3.)*M_PI*(CUBE(bin_edges[i+1])-CUBE(bin_edges[i]))*ndens*npoints;
    printf("pair counts between (%lf, %lf] = %ld\n",bin_edges[i],bin_edges[i+1],pcounts[i]);
    if(testing)
      printf("(naive) pair counts between (%lf, %lf] = %ld\n",bin_edges[i],bin_edges[i+1],pcounts_naive[i]);
    printf("expected pair counts between (%lf, %lf] = %lf\n\n",bin_edges[i],bin_edges[i+1],exp_counts);
  }

  my_free(pcounts);
  my_free(bin_edges);
  my_free(bin_edges_sq);
  
  free_hash(grid);
  return 0;
}
