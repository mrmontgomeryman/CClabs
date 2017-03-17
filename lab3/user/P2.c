#include "P2.h"

void main_P2() {
  while( 1 ) {
    write( STDOUT_FILENO, "P2", 2 ); yield();
  }

  exit( EXIT_SUCCESS );
}
