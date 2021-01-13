#include <assert.h>
#include <stdio.h>
#include <unistd.h>
#include <sys/select.h>
#include <sys/time.h>
#include <time.h>
#include <signal.h>
#include "fsm.h"

#define A_TIME	250
#define B_TIME	30
#define C_TIME	3000

enum semp_state {
  UNO,
  DOS,
  TRES,
};

static int button = 0;
static int button_pressed (fsm_t* this) { return button; }
static int contador = 0;

static int timer = 0;
static void timer_isr (union sigval arg) { timer = 1; }
static void timer_start (int ms)

{
  timer_t timerid;
  struct itimerspec spec;
  struct sigevent se;
  se.sigev_notify = SIGEV_THREAD;
  se.sigev_value.sival_ptr = &timerid;
  se.sigev_notify_function = timer_isr;
  se.sigev_notify_attributes = NULL;
  spec.it_value.tv_sec = ms / 1000;
  spec.it_value.tv_nsec = (ms % 1000) * 1000000;
  spec.it_interval.tv_sec = 0;
  spec.it_interval.tv_nsec = 0;
  timer_create (CLOCK_REALTIME, &se, &timerid);
  timer_settime (timerid, 0, &spec, NULL);
}
static int timer_finished (fsm_t* this) { return timer; }


static void func_uno (fsm_t* this)
{
  printf("Estoy en 1\n");
  timer_start (A_TIME);
}

static void func_dos (fsm_t* this)
{
  printf("Estoy en 2\n");
  timer_start (B_TIME);
}

static void func_tres (fsm_t* this)
{
  printf("Estoy en 3\n");
  timer_start (C_TIME);
}

static void finish (fsm_t* this)
{
  printf("Se acabó\n");

}
 
static int cuenta (fsm_t* this)
  {
    printf("Estoy en cuenta\n");
    if (contador<100){
      contador++;
      printf("Cuenta %d\n",contador);
    }
    return 1;
  }





// Explicit FSM description
static fsm_trans_t semp[] = {
  { UNO, button_pressed, DOS,     func_uno    },
  { UNO,     timer_finished, UNO,  func_tres },
  { DOS,  cuenta, TRES,    func_dos   },
  { TRES,    timer_finished, UNO, finish },
  {-1, NULL, -1, NULL },
  };


// Utility functions, should be elsewhere

// res = a - b
void
timeval_sub (struct timeval *res, struct timeval *a, struct timeval *b)
{
  res->tv_sec = a->tv_sec - b->tv_sec;
  res->tv_usec = a->tv_usec - b->tv_usec;
  if (res->tv_usec < 0) {
    --res->tv_sec;
    res->tv_usec += 1000000;
  }
}

// res = a + b
void
timeval_add (struct timeval *res, struct timeval *a, struct timeval *b)
{
  res->tv_sec = a->tv_sec + b->tv_sec
    + a->tv_usec / 1000000 + b->tv_usec / 1000000; 
  res->tv_usec = a->tv_usec % 1000000 + b->tv_usec % 1000000;
}

// wait until next_activation (absolute time)
void delay_until (struct timeval* next_activation)
{
  struct timeval now, timeout;
  gettimeofday (&now, NULL);
  timeval_sub (&timeout, next_activation, &now);
  select (0, NULL, NULL, NULL, &timeout);
}



int main ()
{
  struct timeval clk_period = { 0, 250 * 1000 };
  struct timeval next_activation;

  fsm_t* my_fsm = fsm_new (semp);

    
  gettimeofday (&next_activation, NULL);
  while (1) {
    printf("Bucle principal\n");
    fsm_fire (my_fsm);
    timeval_add (&next_activation, &next_activation, &clk_period);
    delay_until (&next_activation);


  }
  return 1;
}
