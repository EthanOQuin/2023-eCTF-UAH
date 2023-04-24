#include <stdbool.h>
#include <stdint.h>

#include "driverlib/sysctl.h"
#include "hwsec.h"

void lockdown(void) {
  // Disable the I2C modules
  SysCtlPeripheralDisable(SYSCTL_PERIPH_I2C0);
  SysCtlPeripheralDisable(SYSCTL_PERIPH_I2C1);

  // Disable the SPI modules
  SysCtlPeripheralDisable(SYSCTL_PERIPH_SSI0);
  SysCtlPeripheralDisable(SYSCTL_PERIPH_SSI1);
  SysCtlPeripheralDisable(SYSCTL_PERIPH_SSI2);
  SysCtlPeripheralDisable(SYSCTL_PERIPH_SSI3);

  // Disable the USB interface
  SysCtlPeripheralDisable(SYSCTL_PERIPH_USB0);

  // Disable the Ethernet interface
  SysCtlPeripheralDisable(SYSCTL_PERIPH_EMAC0);

  // Disable the CAN modules
  SysCtlPeripheralDisable(SYSCTL_PERIPH_CAN0);
  SysCtlPeripheralDisable(SYSCTL_PERIPH_CAN1);
}
