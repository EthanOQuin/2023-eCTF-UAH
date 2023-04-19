#ifndef hardware_security
#define hardware_security

#include "inc/tm4c123gh6pm.h"  // Include the TM4C123G register definitions
#include "driverlib/sysctl.h"  // Include the TivaWare system control API

// Define the base address and size of the memory region to be secured
#define SECURE_REGION_BASE   0x20000000
#define SECURE_REGION_SIZE   0x400

//This function serves to enforce hardware security for the board.  
//We disable all vulnerabilities as default and then allow a later function to enable ports and pins required to run the program
void lockdown();

#endif