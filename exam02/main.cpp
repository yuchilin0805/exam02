#include "mbed.h"
#include "fsl_port.h"
#include "fsl_gpio.h"
#define UINT14_MAX        16383
// FXOS8700CQ I2C address
#define FXOS8700CQ_SLAVE_ADDR0 (0x1E<<1) // with pins SA0=0, SA1=0
#define FXOS8700CQ_SLAVE_ADDR1 (0x1D<<1) // with pins SA0=1, SA1=0
#define FXOS8700CQ_SLAVE_ADDR2 (0x1C<<1) // with pins SA0=0, SA1=1
#define FXOS8700CQ_SLAVE_ADDR3 (0x1F<<1) // with pins SA0=1, SA1=1
// FXOS8700CQ internal register addresses
#define FXOS8700Q_STATUS 0x00
#define FXOS8700Q_OUT_X_MSB 0x01
#define FXOS8700Q_OUT_Y_MSB 0x03
#define FXOS8700Q_OUT_Z_MSB 0x05
#define FXOS8700Q_M_OUT_X_MSB 0x33
#define FXOS8700Q_M_OUT_Y_MSB 0x35
#define FXOS8700Q_M_OUT_Z_MSB 0x37
#define FXOS8700Q_WHOAMI 0x0D
#define FXOS8700Q_XYZ_DATA_CFG 0x0E
#define FXOS8700Q_CTRL_REG1 0x2A
#define FXOS8700Q_M_CTRL_REG1 0x5B
#define FXOS8700Q_M_CTRL_REG2 0x5C
#define FXOS8700Q_WHOAMI_VAL 0xC7

I2C i2c( PTD9,PTD8);
Serial pc(USBTX, USBRX);
DigitalOut led1(LED1);
int m_addr = FXOS8700CQ_SLAVE_ADDR1;
EventQueue queue(32 * EVENTS_EVENT_SIZE);

InterruptIn sw2(SW2);

Thread tt;
Ticker ledt;
float x[100],y[100],z[100],f[100];
void logger();
void blink();

void FXOS8700CQ_readRegs(int addr, uint8_t * data, int len);
void FXOS8700CQ_writeRegs(uint8_t * data, int len);
void Trig_tilt_event_log(){
    ledt.attach(&blink,0.5);
    queue.call(logger);
    
}


void blink(){
    led1=!led1;
}
int main() {

    pc.baud(115200);

    led1=1;
    sw2.rise(Trig_tilt_event_log);
    
    tt.start(callback(&queue, &EventQueue::dispatch_forever));
    while(1){};
    
}

void FXOS8700CQ_readRegs(int addr, uint8_t * data, int len) {
char t = addr;
i2c.write(m_addr, &t, 1, true);
i2c.read(m_addr, (char *)data, len);
}

void FXOS8700CQ_writeRegs(uint8_t * data, int len) {
i2c.write(m_addr, (char *)data, len);
}
void logger(){
    uint8_t who_am_i, data[2], res[6];
    int16_t acc16;
    float t[3];
   // float angle;
    float x_displacement=0,x_thisdisplacement=0;
    float y_displacement=0,y_thisdisplacement=0;
    float init_x,init_y,init_z;
    int init_flag=0;
    
    

    // Enable the FXOS8700Q
    FXOS8700CQ_readRegs( FXOS8700Q_CTRL_REG1, &data[1], 1);
    data[1] |= 0x01;
    data[0] = FXOS8700Q_CTRL_REG1;
    FXOS8700CQ_writeRegs(data, 2);

    // Get the slave address
    FXOS8700CQ_readRegs(FXOS8700Q_WHOAMI, &who_am_i, 1);

    //pc.printf("Here is %x\r\n", who_am_i);
    
    for(int i=0;i<100;i++) {
        
        FXOS8700CQ_readRegs(FXOS8700Q_OUT_X_MSB, res, 6);

        acc16 = (res[0] << 6) | (res[1] >> 2);
        if (acc16 > UINT14_MAX/2)
            acc16 -= UINT14_MAX;
        t[0] = ((float)acc16) / 4096.0f;

        acc16 = (res[2] << 6) | (res[3] >> 2);
        if (acc16 > UINT14_MAX/2)
            acc16 -= UINT14_MAX;
        t[1] = ((float)acc16) / 4096.0f;

        acc16 = (res[4] << 6) | (res[5] >> 2);
        if (acc16 > UINT14_MAX/2)
            acc16 -= UINT14_MAX;
        t[2] = ((float)acc16) / 4096.0f;

        x[i]=t[0];
        y[i]=t[1];
        z[i]=t[2];
        //float tmp=t[2]/sqrt(t[0]*t[0]+t[1]*t[1]);
        
        if(!init_flag){
            init_x=t[0];
            init_y=t[1];
            init_z=t[2];

            init_flag=1;
        }
        /*if(i!=0){
            float xmove=x[i+1]-x[i];
            float ymove=y[i+1]-y[i];
            x_thisdisplacement=0.5*9.8*xmove*0.1;
            y_thisdisplacement=0.5*9.8*ymove*0.1;
        }*/
        
        x_thisdisplacement=0.5*9.8*10*x[i]*0.1;
        x_displacement+=x_thisdisplacement;
        y_displacement+=y_thisdisplacement;
        //angle = atan(t[2]/sqrt(t[0]*t[0]+t[1]*t[1]));
        //printf("%1.3f\r\n",(init_angle-angle)*180/3.14159265358);
        
        if(x_displacement>5||y_displacement>5){
            f[i]=1;
        }
        else 
            f[i]=0;
        /*pc.printf("x%1.4f\r\n",t[0]);
        pc.printf("y%1.4f\r\n",t[1]);
        pc.printf("z%1.4f\r\n",t[2]);*/
       // pc.printf("d%1.4f\r\n",x_displacement);
        wait(0.1);
    }
    ledt.detach();
    for(int i=0;i<100;i++){
        pc.printf("x%1.3f\n",x[i]);
        pc.printf("y%1.3f\n",y[i]);
        pc.printf("z%1.3f\n",z[i]);
        pc.printf("t%1.3f\n",f[i]);
        wait(0.1);
    }
}