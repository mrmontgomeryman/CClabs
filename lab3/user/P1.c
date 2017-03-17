#include "P1.h"

void main_P1() {
  while( 1 ) {
    write( STDOUT_FILENO, "P1", 2 ); yield();
  }

  exit( EXIT_SUCCESS );
}
