#include <p24FJ128GB206.h>
#include "config.h"
#include "common.h"
#include "usb.h"
#include "pin.h"
#include "uart.h"
#include <stdio.h>
#include "oc.h"
#include "timer.h"

#define HELLO       0   // Vendor request that prints "Hello World!"
#define SET_VALS    1   // Vendor request that receives 2 unsigned integer values
#define GET_VALS    2   // Vendor request that returns 2 unsigned integer values
#define PRINT_VALS  3   // Vendor request that prints 2 unsigned integer values 
#define TEST_FORWARD_GO  4  
#define TEST_STOP   5  
#define PRINT_FB    6
#define REV         7

#define CURRENT_THRESH 32768
#define NUM_SLICES 100

// declare interupt handler
void __attribute__((interrupt)) _CNInterrupt(void); 
void __attribute__((interrupt)) _OC1Interrupt(void);

uint16_t val1, val2;
uint16_t FB_current, FB_direction, tempDir, vemf, vemf_dir;
uint16_t isInv;
uint16_t duty;
int pos, rots, totalPos;

float vemf_delay;

void MeasureVEMF(_TIMER *timer){
    timer_stop(timer);
    vemf=pin_read(&A[1]);
    if (vemf<32704){
        vemf_dir=1;
    }
    else{
        vemf_dir=0;
    }
}

// interupt callback
void __attribute__((interrupt, auto_psv)) _CNInterrupt(void) {
    IFS1bits.CNIF = 0;
    pin_read(&D[0]);
    // vemf_delay=duty/65535.0*.004;
    timer_after(&timer3,vemf_delay,1,MeasureVEMF);
    if(vemf_dir==0){
        pos=pos+1;
        totalPos=totalPos+1;
        if (pos>100){
            rots=rots+1;
            pos=0;
        }
    }
    else{
        pos=pos-1;
        totalPos=totalPos-1;
        if (pos<0){
            rots=rots-1;
            pos=100;
        }
    }

} 

void __attribute__((interrupt, auto_psv)) _OC1Interrupt(void) { 
    IFS0bits.OC1IF = 0; 
    FB_current = pin_read(&A[2]);
    tempDir = pin_read(&A[0]);
    if (tempDir > CURRENT_THRESH) {
        FB_direction=1;
    }
    else{
        FB_direction=0;
    }
}

void VendorRequests(void) {
    WORD temp;

    switch (USB_setup.bRequest) {
        case HELLO:
            printf("Hello World!\n");
            BD[EP0IN].bytecount = 0;    // set EP0 IN byte count to 0 
            BD[EP0IN].status = 0xC8;    // send packet as DATA1, set UOWN bit
            break;
        case SET_VALS:
            val1 = USB_setup.wValue.w;
            val2 = USB_setup.wIndex.w;
            BD[EP0IN].bytecount = 0;    // set EP0 IN byte count to 0 
            BD[EP0IN].status = 0xC8;    // send packet as DATA1, set UOWN bit
            break;
        case GET_VALS:;
            temp.w = val1;
            BD[EP0IN].address[0] = temp.b[0];
            BD[EP0IN].address[1] = temp.b[1];
            temp.w = val2;
            BD[EP0IN].address[2] = temp.b[0];
            BD[EP0IN].address[3] = temp.b[1];
            BD[EP0IN].bytecount = 4;    // set EP0 IN byte count to 4
            BD[EP0IN].status = 0xC8;    // send packet as DATA1, set UOWN bit
            break;            
        case PRINT_VALS:
            printf("val1 = %u, val2 = %u\n", val1,val2);
            BD[EP0IN].bytecount = 0;    // set EP0 IN byte count to 0
            BD[EP0IN].status = 0xC8;    // send packet as DATA1, set UOWN bit
            break;
        case TEST_FORWARD_GO:
            // run motor at 20k for .1 duty cycle to go vorwartz
            val1=0;
            oc_pwm(&oc1, &D[2], &timer1, 250, duty); 
            printf("starting motor test\n");
            BD[EP0IN].bytecount = 0;    // set EP0 IN byte count to 0 
            BD[EP0IN].status = 0xC8;    // send packet as DATA1, set UOWN bit
            break;
        case TEST_STOP:
            // stop the pwm signal
            oc_free(&oc1);
            printf("ending motor test\n");
            BD[EP0IN].bytecount = 0;    // set EP0 IN byte count to 0 
            BD[EP0IN].status = 0xC8;    // send packet as DATA1, set UOWN bit
            break;
        case PRINT_FB:
            printf("\n\nSensing:\n");
            printf("\tFB current = %u\n", FB_current);
            printf("\tcurr direction = %u\n", FB_direction);
            printf("\tposition: %i   rotations: %i\n",pos,rots);
            printf("\tVemf: %u\n", vemf);
            printf("\tVemf dir: %u\n", vemf_dir);
            BD[EP0IN].bytecount = 0;    // set EP0 IN byte count to 0 
            BD[EP0IN].status = 0xC8;    // send packet as DATA1, set UOWN bit
            break;
        case REV:
            if(isInv==0){
                pin_write(&D[5],1);
                pin_write(&D[6],0);
            }
            else{
                pin_write(&D[6],1);
                pin_write(&D[5],0);
            }
            // pin_write(&D[5],!isInv);
            // pin_write(&D[6],isInv);
            printf("setting to direction %u\n", isInv );
            if (isInv==1){
                isInv=0;
            }
            else{
                isInv=1;
            }
            BD[EP0IN].bytecount = 0;    // set EP0 IN byte count to 0 
            BD[EP0IN].status = 0xC8;    // send packet as DATA1, set UOWN bit
            break;
        default:
            USB_error_flags |= 0x01;    // set Request Error Flag
    }
}

void Turn_CW(void){
    pin_write(&D[5],1);
    pin_write(&D[6],0);
    isInv=1;
}

void Turn_CC(void){
    pin_write(&D[6],1);
    pin_write(&D[5],0);
    isInv=0;
}

void VendorRequestsIn(void) {
    switch (USB_request.setup.bRequest) {
        default:
            USB_error_flags |= 0x01;                    // set Request Error Flag
    }
}

void VendorRequestsOut(void) {
    switch (USB_request.setup.bRequest) {
        default:
            USB_error_flags |= 0x01;                    // set Request Error Flag
    }
}

int16_t main(void) {
    init_clock();
    init_uart();
    init_pin();
    init_timer();
    init_oc();

    //enable CNInterupts
    IEC1bits.CNIE = 1;

    // raise an interupt when CN14 changes
    CNEN1bits.CN14IE = 1;
    IEC0bits.OC1IE = 1; 

    //start low
    IFS1bits.CNIF = 0;
    IFS0bits.OC1IF = 0; 

    // counter
    val1=0;
    FB_current=0;
    pos=0;
    rots=0;
    totalPos=0;
    // 
    duty=0000;
    vemf_delay=duty/65535.0*.004;
    vemf_dir=0;

    // !D2
    pin_digitalOut(&D[2]);
    pin_clear(&D[2]); //set to low

    // D1
    pin_digitalOut(&D[3]);
    pin_clear(&D[3]); //set to low

    // ENABLE
    pin_digitalOut(&D[4]);
    pin_set(&D[4]); //set to high

    //IN2 - D5
    pin_digitalOut(&D[5]);
    pin_clear(&D[5]);
    
    //IN1 - D6
    pin_digitalOut(&D[6]);
    pin_set(&D[6]); 

    isInv=0;

    InitUSB();                              // initialize the USB registers and serial interface engine
    while (USB_USWSTAT!=CONFIG_STATE) {     // while the peripheral is not configured...
        ServiceUSB();                       // ...service USB requests
        
    }
    while (1) {
        ServiceUSB();                       // service any pending USB requests  
        duty=abs(((totalPos+200)/500)*40000);
        if (abs(totalPos)>=500){
            duty=40000; //max duty
        }
        if (totalPos<0){
            Turn_CC();
            // printf("turning counter\n\n\n\n\n");
        }
        if (totalPos>0)
        {
            Turn_CW();
            // printf("turning clock\n\n\n\n\n");
        }
        oc_pwm(&oc1, &D[2], &timer1, 250, duty); 


    }
}

// void oc_pwm(_OC *self, _PIN *out, _TIMER *timer, float freq, uint16_t duty)