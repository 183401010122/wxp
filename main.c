#include<reg51.h>
#include<intrins.h>
#include <absacc.h>


typedef unsigned int uint8;
typedef unsigned long int uint16;
typedef unsigned char  uchar;

uchar  step_motor[] = {0x09,0x08,0x0c,0x04,0x06,0x02,0x03,0x01}; 
uchar code zi_buf_L[5][16]={{0x00,0x00,0x00,0x00,0x00,0x00,0xfe,0xfe,0xfe,0xfe,0x00,0x00,0x00,0x00,0x00,0x00},
                             {0x00,0x00,0x80,0xc0,0xe0,0xf0,0xf8,0xfc,0xfc,0xf8,0xf0,0xe0,0xc0,0x80,0x00,0x00},
														 {0x00,0x00,0x80,0xc0,0xe0,0xf0,0xf8,0xfc,0xc0,0xc0,0xc0,0xc0,0xc0,0xc0,0xc0,0x00},
														 {0x00,0xc0,0xc0,0xc0,0xc0,0xc0,0xc0,0xc0,0xfc,0xf8,0xf0,0xe0,0xc0,0x80,0x00,0x00},
                             {0x00,0x00,0x70,0x70,0x70,0x70,0x70,0x70,0x70,0x70,0x70,0x70,0x70,0x70,0x00,0x00},
                             
                             };
uchar code zi_buf_H[5][16]={{0x00,0x00,0x01,0x03,0x07,0x0f,0x1f,0x3f,0x3f,0x1f,0x0f,0x07,0x03,0x01,0x00,0x00},
                             {0x00,0x00,0x00,0x00,0x00,0x00,0x7f,0x7f,0x7f,0x7f,0x00,0x00,0x00,0x00,0x00,0x00},	 
														 {0x00,0x00,0x01,0x03,0x07,0x0f,0x1f,0x3f,0x03,0x03,0x03,0x03,0x03,0x03,0x03,0x00},
														 {0x00,0x03,0x03,0x03,0x03,0x03,0x03,0x03,0x3f,0x1f,0x0f,0x07,0x03,0x01,0x00,0x00},
                             {0x00,0x00,0x0e,0x0e,0x0e,0x0e,0x0e,0x0e,0x0e,0x0e,0x0e,0x0e,0x0e,0x0e,0x00,0x00},
                             
                            };
static char step_index; //步进索引数，值为 0－7
static int spcount; //步进电机转速参数计数
static bit turn; //步进电机转动方向 
static char count;
static char keynum=4,keybuf=0; 
unsigned char key_id[4]={0x0e,0x0d,0x0b,0x07};					
 
#define CLCK XBYTE [0x8000]
#define CHCK XBYTE [0x8001]
#define RLCK XBYTE [0x8002]
#define RHCK XBYTE [0x8003]
#define key_column XBYTE [0x9002]
#define key_line XBYTE [0x9001]


/*********延时函数************/
void delay(uint16 ms)
	{   
uchar i,j;    
for(i=ms;i>0;i--)   
for(j=110;j>0;j--);}
	
/*********初始化函数************/
void init(){     
  SCON = 0x50;		        // SCON: 方式 1, 8-bit, 允许接收数据 
	TMOD = 0x21;               // TMOD: 设置定时器1工作在方式2, 8-bit 自动重装
	TH1 = 0xFD;               // TH1:  初始值为0xFD  波特率：9600 晶振频率：11.0592MHz  
	TL1 = 0x0;
	TR1 = 1;                  // TR1:  开启定时器1   
	ET0 = 1; //定时器 0 中断允许
  TH0 = 0xFE;
  TL0 = 0x0C; //设定时每隔 0.5ms 中断一次
  TR0 = 1; //开始计数
	EA  = 1;                  //打开总中断
	ES  = 1;                  //打开串口中断
	}

unsigned char runflag;
void gorun()//步进电机驱动
{ 

P1&=0xf0;
P1 |= step_motor [step_index];

if (turn==0) 
 { 
   step_index++; 
   if (step_index>7) 
     step_index=0; 
 } 
 else 
 { 
   step_index--; 
   if (step_index<0) 
     step_index=7; 
 } 

} 

void time0(void) interrupt 1
{
TH0=0xFE;
TL0=0x0C; // 设定时每隔 0.5ms 中断一次
if(runflag)
{spcount--;
if(spcount<=0)
{
spcount = 110;
gorun(); 
}
}
 count++;
	 if(count>=16)
	 {count=0;
	 CHCK=0xff;}
	 RHCK=0;
	 RLCK=0;
	 
	 if(count<8)
	 {CLCK=~(0x01<<count);}
	 else if(count==8)
	 {CLCK=0xff;CHCK=~(0x01);}	//		CLCK=0xff;CHCK=~(0x01);
	 else if(count<16)
	 {CHCK=~(0x01<<(count-8));}		   //CHCK=~(0x01<<(count-8));

	   RLCK=zi_buf_L[keynum][count];	 
		 RHCK=zi_buf_H[keynum][count];
} 	 

unsigned char NoteNumber(){
	unsigned char i=0,j=0,n=0,m=0;
	for(i=0;i<6;i++){
		key_column=~(0x01<<i);
		
		if((key_line&0x0f)!=0x0f){
			//有按键按下，判断是哪个.
			delay(2);
			if ((key_line&0x0f)!=0x0f) {	  
				keybuf=key_line&0x0f;
				for(j=0;j<4;j++){
					if (keybuf==key_id[j]){
					
						keynum = j*6+i;	 
						return 0;
					}
				}
			}
		}
	}
return 0;
}
unsigned char recv_data;
// 串口中断处理函数 （串口接收到数据，发送数据完毕都可以引起串口中断）
void uart_interrupt(void) interrupt 4 		//也叫串行中断服务程序
{
	if(RI) //接收数据(1字节)完毕，RI会被硬件置1
	{
		RI = 0;            		// 将 接收中断标志位 清零(让串口可以继续接收数据)
		recv_data = SBUF;           	//读取接收到的数据，并存放到data   
	}
} 
/*********主函数************/
void main(void)
{
init();
step_index = 0;
spcount = 0;
while(1)   
{ 
	  NoteNumber();
	if(recv_data!=0)//串口发送1 前进 2后退 3左转 4右转 5刹车
		{keynum=recv_data-1;
		 recv_data=0;
		}
	  switch(keynum)
			{case 0:{runflag=1;turn=1;};break;//前进
		   case 1:{runflag=1;turn=0;};break;//后退		{runflag=0;}
			case 2:{runflag=0;}; break;//左转	{runflag=1;turn=0;}
		   case 3:{runflag=0;};break;//右转
		   case 4:{runflag=0;};break;//刹车
			 default: {keynum=0;};
		  }			
			delay(10);
}
}
