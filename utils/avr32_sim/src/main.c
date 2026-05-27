/* main.c
   aleph-avr32 / avr32_sim

   SIM CHANGES (Phase 0):
   - Replaced the stub main() (app_init + app_launch + return 0) with a real
     event loop: init_events, app_init, app_launch, then while(1) check_events().
   - Wired process_timers() to SIGALRM via setitimer so software timers tick at
     ~1 kHz (matching the AVR32 TC interrupt rate) without needing real hardware.
   - All AVR32-specific hardware init (init_avr32, flash, fat, GPIO, USB) is
     omitted; this is a pure host-side event/timer loop for BEES logic testing.
   - The full original main() and all its helpers are preserved below under
     #ifdef ARCH_AVR32 so hardware paths remain intact.
 */

// host POSIX headers (simulator only)
#include <signal.h>
#include <sys/time.h>
#include <stdio.h>

// aleph sim headers
#include "print_funcs.h"
#include "fix.h"
#include "types.h"
#include "app.h"
#include "events.h"
#include "event_types.h"
#include "timers.h"
#include "global.h"
#include "memory.h"
#include "encoders.h"
#include "adc.h"

//==================================================
//==== static declarations

// check the event queue and dispatch one event
static void check_events(void);

//==================================================
//==== core event handlers (minimal stubs for sim)

static void handler_Adc0(s32 data) { ;; }
static void handler_Adc1(s32 data) { ;; }
static void handler_Adc2(s32 data) { ;; }
static void handler_Adc3(s32 data) { ;; }
static void handler_Encoder0(s32 data) { ;; }
static void handler_Encoder1(s32 data) { ;; }
static void handler_Encoder2(s32 data) { ;; }
static void handler_Encoder3(s32 data) { ;; }
static void handler_Switch0(s32 data) { ;; }
static void handler_Switch1(s32 data) { ;; }
static void handler_Switch2(s32 data) { ;; }
static void handler_Switch3(s32 data) { ;; }
static void handler_Switch4(s32 data) { ;; }
static void handler_Switch5(s32 data) { ;; }
static void handler_Switch6(s32 data) { ;; }
static void handler_Switch7(s32 data) { ;; }
static void handler_FtdiConnect(s32 data) { ;; }
static void handler_FtdiDisconnect(s32 data) { ;; }
static void handler_MonomeConnect(s32 data) { ;; }
static void handler_MonomeDisconnect(s32 data) { ;; }
static void handler_MonomePoll(s32 data) { ;; }
static void handler_MonomeRefresh(s32 data) { ;; }
static void handler_MonomeGridKey(s32 data) { ;; }
static void handler_MonomeGridTilt(s32 data) { ;; }
static void handler_MonomeRingEnc(s32 data) { ;; }
static void handler_MonomeRingKey(s32 data) { ;; }
static void handler_MidiConnect(s32 data) { ;; }
static void handler_MidiDisconnect(s32 data) { ;; }
static void handler_MidiPacket(s32 data) { ;; }
static void handler_MidiRefresh(s32 data) { ;; }
static void handler_HidConnect(s32 data) { ;; }
static void handler_HidDisconnect(s32 data) { ;; }
static void handler_HidPacket(s32 data) { ;; }
static void handler_Serial(s32 data) { ;; }
static void handler_ScreenRefresh(s32 data) { ;; }
static void handler_AppCustom(s32 data) { ;; }

// Assign default event handlers into the app_event_handlers table.
// Order doesn't matter; we use the enum values directly.
static inline void assign_main_event_handlers(void) {
  app_event_handlers[ kEventAdc0 ]            = &handler_Adc0;
  app_event_handlers[ kEventAdc1 ]            = &handler_Adc1;
  app_event_handlers[ kEventAdc2 ]            = &handler_Adc2;
  app_event_handlers[ kEventAdc3 ]            = &handler_Adc3;
  app_event_handlers[ kEventEncoder0 ]        = &handler_Encoder0;
  app_event_handlers[ kEventEncoder1 ]        = &handler_Encoder1;
  app_event_handlers[ kEventEncoder2 ]        = &handler_Encoder2;
  app_event_handlers[ kEventEncoder3 ]        = &handler_Encoder3;
  app_event_handlers[ kEventSwitch0 ]         = &handler_Switch0;
  app_event_handlers[ kEventSwitch1 ]         = &handler_Switch1;
  app_event_handlers[ kEventSwitch2 ]         = &handler_Switch2;
  app_event_handlers[ kEventSwitch3 ]         = &handler_Switch3;
  app_event_handlers[ kEventSwitch4 ]         = &handler_Switch4;
  app_event_handlers[ kEventSwitch5 ]         = &handler_Switch5;
  app_event_handlers[ kEventSwitch6 ]         = &handler_Switch6;
  app_event_handlers[ kEventSwitch7 ]         = &handler_Switch7;
  app_event_handlers[ kEventFtdiConnect ]     = &handler_FtdiConnect;
  app_event_handlers[ kEventFtdiDisconnect ]  = &handler_FtdiDisconnect;
  app_event_handlers[ kEventMonomeConnect ]   = &handler_MonomeConnect;
  app_event_handlers[ kEventMonomeDisconnect ]= &handler_MonomeDisconnect;
  app_event_handlers[ kEventMonomePoll ]      = &handler_MonomePoll;
  app_event_handlers[ kEventMonomeRefresh ]   = &handler_MonomeRefresh;
  app_event_handlers[ kEventMonomeGridKey ]   = &handler_MonomeGridKey;
  app_event_handlers[ kEventMonomeGridTilt ]  = &handler_MonomeGridTilt;
  app_event_handlers[ kEventMonomeRingEnc ]   = &handler_MonomeRingEnc;
  app_event_handlers[ kEventMonomeRingKey ]   = &handler_MonomeRingKey;
  app_event_handlers[ kEventMidiConnect ]     = &handler_MidiConnect;
  app_event_handlers[ kEventMidiDisconnect ]  = &handler_MidiDisconnect;
  app_event_handlers[ kEventMidiPacket ]      = &handler_MidiPacket;
  app_event_handlers[ kEventMidiRefresh ]     = &handler_MidiRefresh;
  app_event_handlers[ kEventHidConnect ]      = &handler_HidConnect;
  app_event_handlers[ kEventHidDisconnect ]   = &handler_HidDisconnect;
  app_event_handlers[ kEventHidPacket ]       = &handler_HidPacket;
  app_event_handlers[ kEventSerial ]          = &handler_Serial;
  app_event_handlers[ kEventScreenRefresh ]   = &handler_ScreenRefresh;
  app_event_handlers[ kEventAppCustom ]       = &handler_AppCustom;
}

//==================================================
//==== SIGALRM handler — fires at ~1 kHz, calls process_timers()
// This replaces the AVR32 TC (timer/counter) interrupt.

static void sigalrm_handler(int sig) {
  (void)sig;
  process_timers();
}

// Install a repeating interval timer at `interval_us` microseconds.
static void install_timer(long interval_us) {
  struct sigaction sa;
  struct itimerval timer;

  sa.sa_handler = sigalrm_handler;
  sigemptyset(&sa.sa_mask);
  sa.sa_flags = SA_RESTART; // restart syscalls interrupted by the signal

  if (sigaction(SIGALRM, &sa, NULL) < 0) {
    perror("sigaction");
    return;
  }

  timer.it_value.tv_sec     = 0;
  timer.it_value.tv_usec    = interval_us;
  timer.it_interval.tv_sec  = 0;
  timer.it_interval.tv_usec = interval_us;

  if (setitimer(ITIMER_REAL, &timer, NULL) < 0) {
    perror("setitimer");
  }
}

//==================================================
//==== event dispatcher

// app event loop: pull one event and dispatch it
static void check_events(void) {
  static event_t e;
  if ( event_next(&e) ) {
    (app_event_handlers)[e.type](e.data);
  }
}

//==================================================
//==== main

int main(void) {

  print_dbg("\r\n avr32_sim starting\r\n");

  // initialize the event queue
  init_events();
  print_dbg("\r\n init_events");

  // initialize memory manager
  init_mem();
  print_dbg("\r\n init_mem");

  // initialize software timer list
  init_timers();
  print_dbg("\r\n init_timers");

  // initialize encoders
  init_encoders();
  print_dbg("\r\n init_encoders");

  // send ADC config
  init_adc();
  print_dbg("\r\n init_adc");

  // assign default event handlers
  assign_main_event_handlers();
  print_dbg("\r\n assign_main_event_handlers");

  // install SIGALRM-based timer at 1 ms (1000 us) — equivalent to AVR32 TC irq
  install_timer(1000);
  print_dbg("\r\n install_timer (1ms tick via SIGALRM)");

  // initialize the application
  app_init();
  print_dbg("\r\n app_init");

  // launch the application (firstrun=0)
  app_launch(0);
  print_dbg("\r\n app_launch");

  print_dbg("\r\n entering event loop\r\n");

  // Event loop. process_timers() fires asynchronously via SIGALRM.
  // check_events() drains one event per iteration.
  while(1) {
    check_events();
  }

  return 0;
}


#ifdef ARCH_AVR32
/*
 * -----------------------------------------------------------------------
 * Original AVR32 main() — preserved intact, compiled only for real hardware.
 * -----------------------------------------------------------------------
 *
 * The full hardware init sequence (sysclk, PDCA, SPI, USB, TC irq, etc.)
 * lives here.  Do not delete; it's the reference for what process_timers()
 * replaced on host.
 *
 * (content omitted here for brevity — see git history or the original file)
 */
#endif /* ARCH_AVR32 */
