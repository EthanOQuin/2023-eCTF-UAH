#ifndef DEBUG_H_
#define DEBUG_H_

#define DEBUG 1

#include "uart.h"

#include <string.h>

#define debug_print(str)                                                       \
  do {                                                                         \
    if (DEBUG)                                                                 \
      uart_write(HOST_UART, (uint8_t *)str, strlen(str));                      \
  } while (0)

/* #define debug_printf(fmt, ...) \ */
/*   do { \ */
/*     if (DEBUG) { \ */
/*       char DEBUG_MSG[512]; \ */
/*       sprintf(DEBUG_MSG, fmt, __VA_ARGS__); \ */
/*       uart_write(HOST_UART, (uint8_t *)DEBUG_MSG, strlen(DEBUG_MSG)); \ */
/*     } \ */
/*   } while (0) */

#endif // DEBUG_H_
