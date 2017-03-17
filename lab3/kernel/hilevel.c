#include "hilevel.h"

/* Since we *know* there will be 2 processes, stemming from the 2 user 
 * programs, we can 
 * 
 * - allocate a fixed-size process table (of PCBs), and use a pointer
 *   to keep track of which entry is currently executing, and
 * - employ a fixed-case of round-robin scheduling: no more processes
 *   can be created, and neither is able to complete.
 */

pcb_t pcb[ 2 ], *current = NULL;

void scheduler( ctx_t* ctx ) {
  if      ( current == &pcb[ 0 ] ) {
    memcpy( &pcb[ 0 ].ctx, ctx, sizeof( ctx_t ) ); // preserve P_1
    memcpy( ctx, &pcb[ 1 ].ctx, sizeof( ctx_t ) ); // restore  P_2
    current = &pcb[ 1 ];                           
  }
  else if ( current == &pcb[ 1 ] ) {
    memcpy( &pcb[ 1 ].ctx, ctx, sizeof( ctx_t ) ); // preserve P_2
    memcpy( ctx, &pcb[ 0 ].ctx, sizeof( ctx_t ) ); // restore  P_1
    current = &pcb[ 0 ];
  }

  return;
}

extern void     main_P1(); 
extern uint32_t tos_P1;
extern void     main_P2(); 
extern uint32_t tos_P2;

void hilevel_handler_rst( ctx_t* ctx              ) { 
  /* Initialise PCBs representing processes stemming from execution of
   * the two user programs.  Note in each case that
   *    
   * - the CPSR value of 0x50 means the processor is switched into USR 
   *   mode, with IRQ interrupts enabled, and
   * - the PC and SP values matche the entry point and top of stack. 
   */

  memset( &pcb[ 0 ], 0, sizeof( pcb_t ) );
  pcb[ 0 ].pid      = 1;
  pcb[ 0 ].ctx.cpsr = 0x50;
  pcb[ 0 ].ctx.pc   = ( uint32_t )( &main_P1 );
  pcb[ 0 ].ctx.sp   = ( uint32_t )( &tos_P1  );

  memset( &pcb[ 1 ], 0, sizeof( pcb_t ) );
  pcb[ 1 ].pid      = 2;
  pcb[ 1 ].ctx.cpsr = 0x50;
  pcb[ 1 ].ctx.pc   = ( uint32_t )( &main_P2 );
  pcb[ 1 ].ctx.sp   = ( uint32_t )( &tos_P2  );

  /* Once the PCBs are initialised, we (arbitrarily) select one to be
   * restored (i.e., executed) when the function then returns.
   */

  current = &pcb[ 0 ]; memcpy( ctx, &current->ctx, sizeof( ctx_t ) );

  return;
}

void hilevel_handler_svc( ctx_t* ctx, uint32_t id ) { 
  /* Based on the identified encoded as an immediate operand in the
   * instruction, 
   *
   * - read  the arguments from preserved usr mode registers,
   * - perform whatever is appropriate for this system call,
   * - write any return value back to preserved usr mode registers.
   */

  switch( id ) {
    case 0x00 : { // 0x00 => yield()
      scheduler( ctx );
      break;
    }
    case 0x01 : { // 0x01 => write( fd, x, n )
      int   fd = ( int   )( ctx->gpr[ 0 ] );  
      char*  x = ( char* )( ctx->gpr[ 1 ] );  
      int    n = ( int   )( ctx->gpr[ 2 ] ); 

      for( int i = 0; i < n; i++ ) {
        PL011_putc( UART0, *x++, true );
      }
      
      ctx->gpr[ 0 ] = n;
      break;
    }
    default   : { // 0x?? => unknown/unsupported
      break;
    }
  }

  return;
}
