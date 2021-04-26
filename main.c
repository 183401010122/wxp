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
static char step_index; //������������ֵΪ 0��7
static int spcount; //�������ת�ٲ�������
static bit turn; //�������ת������ 
static char count;
static char keynum=4,keybuf=0; 
unsigned char key_id[4]={0x0e,0x0d,0x0b,0x07};					
 
#define CLCK XBYTE [0x8000]
#define CHCK XBYTE [0x8001]
#define RLCK XBYTE [0x8002]
#define RHCK XBYTE [0x8003]
#define key_column XBYTE [0x9002]
#define key_line XBYTE [0x9001]


/*********��ʱ����************/
void delay(uint16 ms)
	{   
uchar i,j;    
for(i=ms;i>0;i--)   
for(j=110;j>0;j--);}
	
/*********��ʼ������************/
void init(){     
  SCON = 0x50;		        // SCON: ��ʽ 1, 8-bit, ����������� 
	TMOD = 0x21;               // TMOD: ���ö�ʱ��1�����ڷ�ʽ2, 8-bit �Զ���װ
	TH1 = 0xFD;               // TH1:  ��ʼֵΪ0xFD  �����ʣ�9600 ����Ƶ�ʣ�11.0592MHz  
	TL1 = 0x0;
	TR1 = 1;                  // TR1:  ������ʱ��1   
	ET0 = 1; //��ʱ�� 0 �ж�����
  TH0 = 0xFE;
  TL0 = 0x0C; //�趨ʱÿ�� 0.5ms �ж�һ��
  TR0 = 1; //��ʼ����
	EA  = 1;                  //�����ж�
	ES  = 1;                  //�򿪴����ж�
	}

unsigned char runflag;
void gorun()//�����������
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
TL0=0x0C; // �趨ʱÿ�� 0.5ms �ж�һ��
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
			//�а������£��ж����ĸ�.
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
// �����жϴ����� �����ڽ��յ����ݣ�����������϶��������𴮿��жϣ�
void uart_interrupt(void) interrupt 4 		//Ҳ�д����жϷ������
{
	if(RI) //��������(1�ֽ�)��ϣ�RI�ᱻӲ����1
	{
		RI = 0;            		// �� �����жϱ�־λ ����(�ô��ڿ��Լ�����������)
		recv_data = SBUF;           	//��ȡ���յ������ݣ�����ŵ�data   
	}
} 
/*********������************/
void main(void)
{
init();
step_index = 0;
spcount = 0;
while(1)   
{ 
	  NoteNumber();
	if(recv_data!=0)//���ڷ���1 ǰ�� 2���� 3��ת 4��ת 5ɲ��
		{keynum=recv_data-1;
		 recv_data=0;
		}
	  switch(keynum)
			{case 0:{runflag=1;turn=1;};break;//ǰ��
		   case 1:{runflag=1;turn=0;};break;//����		{runflag=0;}
			case 2:{runflag=0;}; break;//��ת	{runflag=1;turn=0;}
		   case 3:{runflag=0;};break;//��ת
		   case 4:{runflag=0;};break;//ɲ��
			 default: {keynum=0;};
		  }			
			delay(10);
}
}
