#include "hardware_security.h"  // Include the header file
#include "driverlib/sysctl.h"  // Include the TivaWare system control API

// Define the base address and size of the memory region to be secured
#define SECURE_REGION_BASE   0x20000000
#define SECURE_REGION_SIZE   0x400

//This function serves to enforce hardware security for the board.  
//We disable all vulnerabilities as default and then allow a later function to enable ports and pins required to run the program
void lockdown()
{
    // 1. Disable debugging interfaces
    // Disable JTAG and SWD debugging without interrupting a program
    SYSCTL_GPIOHBCTL_R = 0;
    GPIO_PORTA_LOCK_R = GPIO_LOCK_KEY;
    GPIO_PORTA_CR_R |= GPIO_PIN_0 | GPIO_PIN_1;
    GPIO_PORTA_AFSEL_R &= ~(GPIO_PIN_0 | GPIO_PIN_1);
    GPIO_PORTA_PCTL_R &= ~(GPIO_PCTL_PA0_M | GPIO_PCTL_PA1_M);
    GPIO_PORTA_AMSEL_R &= ~(GPIO_PIN_0 | GPIO_PIN_1);
    GPIO_PORTA_DEN_R |= GPIO_PIN_0 | GPIO_PIN_1;

    // 2. Initialize system clock and GPIO pins
    // Initialize the system clock to run at 80 MHz
    SysCtlClockSet(SYSCTL_SYSDIV_2_5 | SYSCTL_USE_PLL | SYSCTL_XTAL_16MHZ | SYSCTL_OSC_MAIN);

    // Enable GPIOF peripheral
    SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOF);

    // Configure PF4 as input pin
    GPIOPinTypeGPIOInput(GPIO_PORTF_BASE, GPIO_PIN_4);

    // Enable pull-up resistor for PF4
    GPIOPadConfigSet(GPIO_PORTF_BASE, GPIO_PIN_4, GPIO_STRENGTH_2MA, GPIO_PIN_TYPE_STD_WPU);

    // 3. Disable built-in peripherals
    // Disable the ADC module
    SysCtlPeripheralDisable(SYSCTL_PERIPH_ADC0);

    // Disable the timers
    SysCtlPeripheralDisable(SYSCTL_PERIPH_TIMER0);
    SysCtlPeripheralDisable(SYSCTL_PERIPH_TIMER1);
    SysCtlPeripheralDisable(SYSCTL_PERIPH_TIMER2);
    SysCtlPeripheralDisable(SYSCTL_PERIPH_TIMER3);

    // Disable the PWM modules
    SysCtlPeripheralDisable(SYSCTL_PERIPH_PWM0);
    SysCtlPeripheralDisable(SYSCTL_PERIPH_PWM1);

    // 4. Disable communication interfaces
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

    // 5. Secure memory regions
    // Disable the MPU before configuring it
    MPU_CTRL &= ~MPU_CTRL_ENABLE;

    // Configure a region to protect the secure memory region
    MPU_RNR = 0;  // Set the region number to 0
    MPU_RBAR = SECURE_REGION_BASE | MPU_RBAR_VALID | MPU_RBAR_REGION(0);
    MPU_RASR = MPU_RASR_ENABLE | MPU_RASR_SIZE(SECURE_REGION_SIZE) |MPU_RASR_ATTR_NORMAL_WB_WA | MPU_RASR_AP_PRIV_RW_USER_NO | MPU_RASR_XN;
    
    // Enable the MPU with the default settings
    MPU_CTRL = MPU_CTRL_PRIVDEFENA | MPU_CTRL_ENABLE;
}