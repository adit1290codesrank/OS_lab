#include "types.h"
#include "stat.h"
#include "user.h"

#define MAXF 10
#define MAXR 64

// default reference string from the assignment
int defrefs[] = {7, 0, 1, 2, 0, 3, 0, 4, 2, 3, 0, 3, 2, 1, 2, 0, 1, 7, 0, 1};

// simulate FIFO (lru = 0) or LRU (lru = 1) with nf frames, return number of faults
// verbose = 1 prints the frames and hit/fault for every reference
int
simulate(int *refs, int n, int nf, int lru, int verbose)
{
  int frame[MAXF];   // page held by each frame, -1 = empty
  int last[MAXF];    // LRU: time of last use of each frame, -1 = empty
  int next = 0;      // FIFO: frame holding the oldest page
  int faults = 0;
  int t, k, hit, victim;

  for(k = 0; k < nf; k++){
    frame[k] = -1;
    last[k] = -1;
  }

  if(verbose)
    printf(1, "\n--- %s, %d frames ---\n", lru ? "LRU" : "FIFO", nf);

  for(t = 0; t < n; t++){
    hit = -1;
    for(k = 0; k < nf; k++)          // is the page already in a frame?
      if(frame[k] == refs[t])
        hit = k;

    if(hit >= 0){
      if(lru)
        last[hit] = t;               // LRU: hit refreshes the page's last-use time
    } else {
      faults++;
      if(lru){                       // LRU: evict the least recently used (empty frames have -1)
        victim = 0;
        for(k = 1; k < nf; k++)
          if(last[k] < last[victim])
            victim = k;
      } else {                       // FIFO: evict the oldest page, frames are filled in order
        victim = next;
        next = (next + 1) % nf;
      }
      frame[victim] = refs[t];
      last[victim] = t;
    }

    if(verbose){
      printf(1, "ref %d :", refs[t]);
      for(k = 0; k < nf; k++){
        if(frame[k] == -1)
          printf(1, "  -");
        else
          printf(1, "  %d", frame[k]);
      }
      printf(1, "   %s\n", hit >= 0 ? "hit" : "FAULT");
    }
  }

  if(verbose)
    printf(1, "%s total page faults: %d\n", lru ? "LRU" : "FIFO", faults);
  return faults;
}

// print hits/n as a percentage with 2 decimals (xv6 printf has no %f)
void
printratio(int hits, int n)
{
  int r = hits * 10000 / n;
  printf(1, "%s%d.%d%d%%", r < 1000 ? " " : "", r / 100, (r / 10) % 10, r % 10);
}

int
main(int argc, char *argv[])
{
  int refs[MAXR];
  int n, nf, i, f, ff, lf;
  char *p;

  // usage: pagerepl [frames ref1 ref2 ... | frames ref1,ref2,...]; no arguments = 3 frames + default string
  if(argc == 1){
    nf = 3;
    n = sizeof(defrefs) / sizeof(defrefs[0]);
    for(i = 0; i < n; i++)
      refs[i] = defrefs[i];
  } else {
    if(argc < 3){
      printf(2, "usage: pagerepl frames ref1 ref2 ...  or  pagerepl frames ref1,ref2,...\n");
      exit();
    }
    nf = atoi(argv[1]);
    if(nf < 1 || nf > MAXF){
      printf(2, "pagerepl: frames must be 1..%d\n", MAXF);
      exit();
    }
    // refs may be separate arguments or comma-separated (xv6 sh allows only 10 args)
    n = 0;
    for(i = 2; i < argc; i++){
      for(p = argv[i]; *p; ){
        if(*p < '0' || *p > '9'){
          p++;
          continue;
        }
        if(n == MAXR){
          printf(2, "pagerepl: at most %d references\n", MAXR);
          exit();
        }
        refs[n] = 0;
        while(*p >= '0' && *p <= '9')
          refs[n] = refs[n] * 10 + (*p++ - '0');
        n++;
      }
    }
    if(n == 0){
      printf(2, "pagerepl: empty reference string\n");
      exit();
    }
  }

  printf(1, "reference string (%d refs):", n);
  for(i = 0; i < n; i++)
    printf(1, " %d", refs[i]);
  printf(1, "\n");

  // (a), (b): step-by-step trace for the chosen number of frames
  simulate(refs, n, nf, 0, 1);
  simulate(refs, n, nf, 1, 1);

  // (c): summary table for 3, 4 and 5 frames
  printf(1, "\nframes | FIFO faults | FIFO hit ratio | LRU faults | LRU hit ratio\n");
  printf(1, "-------+-------------+----------------+------------+--------------\n");
  for(f = 3; f <= 5; f++){
    ff = simulate(refs, n, f, 0, 0);
    lf = simulate(refs, n, f, 1, 0);
    printf(1, "   %d   |     %s%d      |     ", f, ff < 10 ? " " : "", ff);
    printratio(n - ff, n);
    printf(1, "     |     %s%d     |    ", lf < 10 ? " " : "", lf);
    printratio(n - lf, n);
    printf(1, "\n");
  }
  exit();
}
