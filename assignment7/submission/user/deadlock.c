#include "types.h"
#include "stat.h"
#include "user.h"

#define MAXP 5
#define MAXR 3

int n, m;
int avail[MAXR];
int alloc[MAXP][MAXR];
int request[MAXP][MAXR];
int max[MAXP][MAXR];
int need[MAXP][MAXR];
char *pname[MAXP];

void
printvec(char *label, int *v)
{
  int j;
  printf(1, "%s", label);
  for(j = 0; j < m; j++)
    printf(1, " %d", v[j]);
  printf(1, "\n");
}

void
printmat(char *label, int mat[MAXP][MAXR])
{
  int i, j;
  printf(1, "%s\n", label);
  for(i = 0; i < n; i++){
    printf(1, "  %s:", pname[i]);
    for(j = 0; j < m; j++)
      printf(1, " %d", mat[i][j]);
    printf(1, "\n");
  }
}

int
fits(int *a, int *b)
{
  int j;
  for(j = 0; j < m; j++)
    if(a[j] > b[j])
      return 0;
  return 1;
}

int
detect_deadlock(int *deadlocked)
{
  int work[MAXR], finish[MAXP];
  int i, j, progress, nd;

  for(j = 0; j < m; j++)
    work[j] = avail[j];
  for(i = 0; i < n; i++){
    finish[i] = 1;
    for(j = 0; j < m; j++)
      if(alloc[i][j] != 0)
        finish[i] = 0;
  }
  do {
    progress = 0;
    for(i = 0; i < n; i++){
      if(!finish[i] && fits(request[i], work)){
        for(j = 0; j < m; j++)
          work[j] += alloc[i][j];
        finish[i] = 1;
        progress = 1;
        printf(1, "  %s request satisfiable -> finishes,", pname[i]);
        printvec(" Work =", work);
      }
    }
  } while(progress);

  nd = 0;
  for(i = 0; i < n; i++)
    if(!finish[i])
      deadlocked[nd++] = i;
  return nd;
}

int
is_safe_state(void)
{
  int work[MAXR], finish[MAXP], seq[MAXP];
  int i, j, k, progress;

  for(j = 0; j < m; j++)
    work[j] = avail[j];
  for(i = 0; i < n; i++)
    finish[i] = 0;
  k = 0;
  do {
    progress = 0;
    for(i = 0; i < n; i++){
      if(!finish[i] && fits(need[i], work)){
        for(j = 0; j < m; j++)
          work[j] += alloc[i][j];
        finish[i] = 1;
        seq[k++] = i;
        progress = 1;
      }
    }
  } while(progress);

  if(k < n){
    printf(1, "  UNSAFE: no safe sequence exists (only %d of %d can finish)\n", k, n);
    return 0;
  }
  printf(1, "  SAFE: safe sequence <");
  for(i = 0; i < n; i++)
    printf(1, " %s", pname[seq[i]]);
  printf(1, " >\n");
  return 1;
}

void
compute_need(void)
{
  int i, j;
  for(i = 0; i < n; i++)
    for(j = 0; j < m; j++)
      need[i][j] = max[i][j] - alloc[i][j];
}

void
print_banker_state(void)
{
  printvec("Available:", avail);
  printmat("Allocation:", alloc);
  printmat("Max:", max);
  printmat("Need:", need);
}

int
request_resources(int p, int *req)
{
  int j, safe;

  printf(1, "\n%s requests", pname[p]);
  printvec("", req);
  if(!fits(req, need[p])){
    printf(1, "  denied: request exceeds declared Need\n");
    return 0;
  }
  if(!fits(req, avail)){
    printf(1, "  denied: not enough Available, %s must wait\n", pname[p]);
    return 0;
  }
  for(j = 0; j < m; j++){
    avail[j] -= req[j];
    alloc[p][j] += req[j];
    need[p][j] -= req[j];
  }
  printf(1, "  pretend-allocated, state would be:\n");
  print_banker_state();
  safe = is_safe_state();
  if(!safe){
    for(j = 0; j < m; j++){
      avail[j] += req[j];
      alloc[p][j] -= req[j];
      need[p][j] += req[j];
    }
    printf(1, "  -> request DENIED (would lead to unsafe state), rolled back\n");
  } else {
    printf(1, "  -> request GRANTED\n");
  }
  return safe;
}

void
load(int np, int nr, int *a, int (*al)[MAXR], int (*rq)[MAXR])
{
  int i, j;
  n = np;
  m = nr;
  for(j = 0; j < m; j++)
    avail[j] = a[j];
  for(i = 0; i < n; i++)
    for(j = 0; j < m; j++){
      alloc[i][j] = al[i][j];
      request[i][j] = rq[i][j];
      max[i][j] = rq[i][j];
    }
}

void
run_detection(char *title)
{
  int dl[MAXP], nd, i;

  printf(1, "\n==== %s ====\n", title);
  printvec("Available:", avail);
  printmat("Allocation:", alloc);
  printmat("Request:", request);
  printf(1, "Running detection algorithm:\n");
  nd = detect_deadlock(dl);
  if(nd == 0)
    printf(1, "RESULT: no deadlock\n");
  else {
    printf(1, "RESULT: DEADLOCK among");
    for(i = 0; i < nd; i++)
      printf(1, " %s", pname[dl[i]]);
    printf(1, "\n");
  }
}

int a0[MAXR] = {0, 0, 0};
int al0[MAXP][MAXR] = {{0,1,0},{2,0,0},{3,0,3},{2,1,1},{0,0,2}};
int rq1[MAXP][MAXR] = {{0,0,0},{2,0,2},{0,0,0},{1,0,0},{0,0,2}};
int rq2[MAXP][MAXR] = {{0,0,0},{2,0,2},{0,0,1},{1,0,0},{0,0,2}};

int ab[MAXR] = {3, 3, 2};
int alb[MAXP][MAXR] = {{0,1,0},{2,0,0},{3,0,2},{2,1,1},{0,0,2}};
int mxb[MAXP][MAXR] = {{7,5,3},{3,2,2},{9,0,2},{2,2,2},{4,3,3}};

void
pipe_deadlock(void)
{
  int p1[2], p2[2], st[2];
  int pa, pb, dl[MAXP], nd, i;
  char c;
  int pa0[MAXR] = {0, 0};
  int pal[MAXP][MAXR] = {{1,0},{0,1}};
  int prq[MAXP][MAXR] = {{0,1},{1,0}};

  printf(1, "\n==== Test 5: real deadlock with xv6 processes and pipes ====\n");
  pipe(p1);
  pipe(p2);
  pipe(st);

  if((pa = fork()) == 0){
    close(p1[0]);
    close(p2[1]);
    close(st[0]);
    printf(1, "  A (pid %d): holds Pipe1 write end, waiting to read Pipe2\n", getpid());
    write(st[1], "A", 1);
    read(p2[0], &c, 1);
    printf(1, "  A: unblocked (should never happen)\n");
    exit();
  }
  read(st[0], &c, 1);
  if((pb = fork()) == 0){
    close(p2[0]);
    close(p1[1]);
    close(st[0]);
    printf(1, "  B (pid %d): holds Pipe2 write end, waiting to read Pipe1\n", getpid());
    write(st[1], "B", 1);
    read(p1[0], &c, 1);
    printf(1, "  B: unblocked (should never happen)\n");
    exit();
  }
  close(p1[0]); close(p1[1]);
  close(p2[0]); close(p2[1]);
  close(st[1]);
  read(st[0], &c, 1);
  sleep(100);
  printf(1, "  after 100 ticks neither A nor B has made progress\n");

  pname[0] = "A";
  pname[1] = "B";
  load(2, 2, pa0, pal, prq);
  printf(1, "Resources: R0=Pipe1 (write end), R1=Pipe2 (write end)\n");
  printvec("Available:", avail);
  printmat("Allocation:", alloc);
  printmat("Request:", request);
  printf(1, "Running detection algorithm:\n");
  nd = detect_deadlock(dl);
  if(nd == 0)
    printf(1, "RESULT: no deadlock\n");
  else {
    printf(1, "RESULT: DEADLOCK (circular wait) among");
    for(i = 0; i < nd; i++)
      printf(1, " %s", pname[dl[i]]);
    printf(1, "\n");
  }
  kill(pa);
  kill(pb);
  wait();
  wait();
  close(st[0]);
  printf(1, "  killed pids %d and %d to break the deadlock\n", pa, pb);
}

int
main(int argc, char *argv[])
{
  int r1[MAXR] = {1, 0, 2};
  int r2[MAXR] = {0, 2, 0};

  pname[0] = "P0"; pname[1] = "P1"; pname[2] = "P2"; pname[3] = "P3"; pname[4] = "P4";

  load(5, 3, a0, al0, rq1);
  run_detection("Test 1: detection, expected NO deadlock");

  load(5, 3, a0, al0, rq2);
  run_detection("Test 2: detection, P2 asks for one more C, expected DEADLOCK");

  printf(1, "\n==== Test 3: Banker's algorithm, safe state ====\n");
  load(5, 3, ab, alb, mxb);
  compute_need();
  print_banker_state();
  is_safe_state();
  request_resources(1, r1);

  printf(1, "\n==== Test 4: Banker's algorithm, request that makes state unsafe ====\n");
  request_resources(0, r2);

  pipe_deadlock();
  exit();
}
