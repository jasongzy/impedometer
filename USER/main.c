#include "stm32_config.h"
#include "stdio.h"
#include "led.h"
#include "lcd.h"
#include "key.h"
#include "AD9910.h"
//#include "task_manage.h"
//#include "timer.h"
#include "sys.h"
#include "ascii_convert.h"
#include "adc.h"

extern uchar cfr2[4]; //cfr2������
extern uchar cfr1[4]; //cfr1������

int main(void)
{
	u16 i = 0;
	u16 j = 0;
	u16 adcx;
	float temp;
	float volt;
	float buff[150];
	u8 key;
	u16 times = 0;
	u8 flag = 0;
	u8 adcout = 0;
	u16 len;
	ulong ctrl[4];
	NVIC_PriorityGroupConfig(NVIC_PriorityGroup_2); //�����жϷ���
	//uart_init(115200);	 //���ڳ�ʼ��Ϊ115200
	uart_init(9600);
	delay_init(72); //��ʼ����ʱ����
	LED_Init();		  //��ʼ��LED�ӿ�
	KEY_Init();		  //������ʼ��
	LCD_Init();		  //Һ����ʼ��
	//LCD_Clear();
	Adc_Init();		  //ADC��ʼ��
	delay_ms(300);
	
	Init_ad9910();	//9910��ʼ��
	
	POINT_COLOR=RED;//��������Ϊ��ɫ 
	LCD_ShowString(60,50,200,16,16,(u8*)"Elite STM32");	
	LCD_ShowString(60,70,200,16,16,(u8*)"ADC TEST");	
	LCD_ShowString(60,90,200,16,16,(u8*)"ATOM@ALIENTEK");
	LCD_ShowString(60,110,200,16,16,(u8*)"2015/1/14");	
	//��ʾ��ʾ��Ϣ
	POINT_COLOR=BLUE;//��������Ϊ��ɫ
	LCD_ShowString(60,130,200,16,16,(u8*)"ADC_CH0_VAL:");	      
	LCD_ShowString(60,150,200,16,16,(u8*)"ADC_CH0_VOL:0.000V");
	
	cfr1[0] = 0x00;		 //RAM ʧ��
	cfr2[1] = 0x00;		 //DRG ʧ��
	Txcfr();			 //����cfrx������
	Amp_convert(200);	//д���ȣ����뷶Χ��1-650 mV
	Freq_convert(20000); //дƵ�ʣ����뷶Χ��1-400 000 000Hz
	
	while (1)
	{
		times++;
		if (times % 30 == 0)
			LED0 = !LED0; //��˸LED,��ʾϵͳ��������.
		delay_ms(10);
		
		//********************************************************************************
		//LCD��Ļ��ʾ
		//adcx=Get_Adc_Average(ADC_Channel_1,10);
		adcx=Get_Adc(ADC_Channel_1);
		LCD_ShowxNum(156,130,adcx,4,16,0);//��ʾADC��ֵ
		temp=(float)adcx*(3.3/4096);
		volt = temp;
		adcx=temp;
		LCD_ShowxNum(156,150,adcx,1,16,0);//��ʾ��ѹֵ
 		temp-=adcx;
		temp*=1000;
		LCD_ShowxNum(172,150,temp,3,16,0X80);
		LED0=!LED0;
		delay_ms(100);
		
		//ADC��¼
		if(i<150 && adcout)
		{ 
			buff[i]=volt;
			printf("%f\r\n",buff[i]);
			++i;
		}
		else
		{
			adcout=0;
			i=0;
		}
		
		//********************************************************************************
		//�������ݽ��մ���
		if (USART_RX_STA & 0x8000)
		{
			LED1=!LED1;
			len = USART_RX_STA & 0x3fff; //�õ��˴ν��յ������ݳ���
			if (USART_RX_BUF[0] == 0x41) //������������ʼ��"A",���ң�e.g. A20#B200#!��
			{
				cfr1[0] = 0x00; //RAM ʧ��
				cfr2[1] = 0x00; //DRG ʧ��
				Txcfr();		//����cfrx������
				//�����������
				for (i = 0; USART_RX_BUF[i] != 0x42 && i < len; i++);
				if (i == len) //δ�ҵ���2����ʼ��
				{
					printf("Error!\r\n");
					USART_RX_STA = 0;
					continue;
				}
				ctrl[0]=toint(USART_RX_BUF, i);
				ctrl[1]=toint(USART_RX_BUF + i, len - i);
				Freq_convert(1000*ctrl[0]);
				Amp_convert(ctrl[1]);
				printf("���ڿ��ƣ������źţ�%lu kHz / %lu mV��\r\n",ctrl[0],ctrl[1]);
				//LCD_ShowString(60,250,200,16,16,(u8*)"Sine");
				//LCD_ShowxNum(60,250,ctrl[0],8,16,1);
				//LCD_ShowxNum(100,250,ctrl[1],8,16,1);
			}
			else if (USART_RX_BUF[0] == 0x43) //������������ʼ��"C",������e.g. C20#!��
			{
				ctrl[0]=toint(USART_RX_BUF, len);
				Square_wave(1000*ctrl[0]);
				printf("���ڿ��ƣ������źţ�%lu kHz��\r\n",ctrl[0]);
				//LCD_ShowString(60,250,200,16,16,(u8*)"Square");
				//LCD_ShowxNum(80,250,ctrl[0],8,16,1);
			}
			else if (USART_RX_BUF[0] == 0x44) //������������ʼ��"D",��ݲ���e.g. D20#!��
			{
				ctrl[0]=toint(USART_RX_BUF, len);
				Sawtooth_wave(1000*ctrl[0]);
				printf("���ڿ��ƣ���ݲ��źţ�%lu kHz��\r\n",ctrl[0]);
				//LCD_ShowString(60,250,200,16,16,(u8*)"Sawtooth");
				//LCD_ShowxNum(80,250,ctrl[0],8,16,1);
			}
			else if (USART_RX_BUF[0] == 0x45) //������������ʼ��"E",ɨƵ��e.g. E100#F100000#G20#H120#!��
			{
				Amp_convert(200);
				//�����������
				for (i = 0; USART_RX_BUF[i] != 0x46 && i < len; i++);
				if (i == len) //δ�ҵ���2����ʼ��
				{
					printf("Error!\r\n");
					USART_RX_STA = 0;
					continue;
				}
				ctrl[0] = toint(USART_RX_BUF, i);
				for (j = i; USART_RX_BUF[j] != 0x47 && j < len; j++);
				if (j == len) //δ�ҵ���3����ʼ��
				{
					printf("Error!\r\n");
					USART_RX_STA = 0;
					continue;
				}
				ctrl[1] = toint(USART_RX_BUF + i, j - i);
				if (ctrl[0] >= ctrl[1]) //���޸�������
				{
					printf("Error!\r\n");
					USART_RX_STA = 0;
					continue;
				}
				for (i = j; USART_RX_BUF[i] != 0x48 && i < len; i++);
				if (i == len) //δ�ҵ���4����ʼ��
				{
					printf("Error!\r\n");
					USART_RX_STA = 0;
					continue;
				}
				ctrl[2] = toint(USART_RX_BUF + j, i - j);
				ctrl[3] = toint(USART_RX_BUF + i, len - i);
				SweepFre(ctrl[0], ctrl[1], ctrl[2], ctrl[3] * 1000);
				printf("���ڿ��ƣ�ɨƵ�źţ�%lu-%lu Hz / %lu Hz / %lu ms��\r\n",ctrl[0],ctrl[1],ctrl[2],ctrl[3]);
				//LCD_ShowString(60,250,200,16,16,(u8*)"Sweep");
			}
			else if (USART_RX_BUF[0] == 0x54) //����ADC��¼
			{
				adcout=1;
			}
			else
			{
				printf("Error!\r\n");
				LCD_ShowString(100,200,200,16,16,(u8*)"Error!");
			}
			LED1=!LED1;
			USART_RX_STA = 0;
		}

		//********************************************************************************
		//��������
		key = KEY_Scan(0); //�õ���ֵ
		if (key)
		{
			LED1=!LED1;
			switch (key)
			{
			case WKUP_PRES:			 //��Ƶ����
				cfr1[0] = 0x00;		 //RAM ʧ��
				cfr2[1] = 0x00;		 //DRG ʧ��
				Txcfr();			 //����cfrx������
				Amp_convert(200);	//д���ȣ����뷶Χ��1-650 mV
				Freq_convert(20000); //дƵ�ʣ����뷶Χ��1-400 000 000Hz
				printf("�������ƣ������ź�\r\n");
				break;
			case KEY0_PRES: //RAM
				if (flag == 0)
				{
					Sawtooth_wave(20000); //��ݲ�������ʱ�������뷶Χ��4*(1~65536)ns
					printf("�������ƣ���ݲ��ź�\r\n");
					flag = !flag;
				}
				else
				{
					Square_wave(20000); //����������ʱ�������뷶Χ��4*(1~65536)ns
					printf("�������ƣ������ź�\r\n");
					flag = !flag;
				}
				break;
			case KEY1_PRES:		  //ɨƵ
				Amp_convert(200); //д���ȣ����뷶Χ��1-650 mV
				//ɨƵ������Ƶ�ʣ�����Ƶ�ʣ�Ƶ�ʲ�������λ��Hz��������ʱ��������λ��us��
				SweepFre(100, 100000, 20, 120000); //����ʱ�䷶Χ��4*(1~65536)ns
				printf("�������ƣ�ɨƵ�ź�\r\n");
				break;
			}
			LED1=!LED1;
		}
	}
}
