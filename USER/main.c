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

extern uchar cfr2[4]; //cfr2控制字
extern uchar cfr1[4]; //cfr1控制字

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
	NVIC_PriorityGroupConfig(NVIC_PriorityGroup_2); //设置中断分组
	//uart_init(115200);	 //串口初始化为115200
	uart_init(9600);
	delay_init(72); //初始化延时函数
	LED_Init();		  //初始化LED接口
	KEY_Init();		  //按键初始化
	LCD_Init();		  //液晶初始化
	//LCD_Clear();
	Adc_Init();		  //ADC初始化
	delay_ms(300);
	
	Init_ad9910();	//9910初始化
	
	POINT_COLOR=RED;//设置字体为红色 
	LCD_ShowString(60,50,200,16,16,(u8*)"Elite STM32");	
	LCD_ShowString(60,70,200,16,16,(u8*)"ADC TEST");	
	LCD_ShowString(60,90,200,16,16,(u8*)"ATOM@ALIENTEK");
	LCD_ShowString(60,110,200,16,16,(u8*)"2015/1/14");	
	//显示提示信息
	POINT_COLOR=BLUE;//设置字体为蓝色
	LCD_ShowString(60,130,200,16,16,(u8*)"ADC_CH0_VAL:");	      
	LCD_ShowString(60,150,200,16,16,(u8*)"ADC_CH0_VOL:0.000V");
	
	cfr1[0] = 0x00;		 //RAM 失能
	cfr2[1] = 0x00;		 //DRG 失能
	Txcfr();			 //发送cfrx控制字
	Amp_convert(200);	//写幅度，输入范围：1-650 mV
	Freq_convert(20000); //写频率，输入范围：1-400 000 000Hz
	
	while (1)
	{
		times++;
		if (times % 30 == 0)
			LED0 = !LED0; //闪烁LED,提示系统正在运行.
		delay_ms(10);
		
		//********************************************************************************
		//LCD屏幕显示
		//adcx=Get_Adc_Average(ADC_Channel_1,10);
		adcx=Get_Adc(ADC_Channel_1);
		LCD_ShowxNum(156,130,adcx,4,16,0);//显示ADC的值
		temp=(float)adcx*(3.3/4096);
		volt = temp;
		adcx=temp;
		LCD_ShowxNum(156,150,adcx,1,16,0);//显示电压值
 		temp-=adcx;
		temp*=1000;
		LCD_ShowxNum(172,150,temp,3,16,0X80);
		LED0=!LED0;
		delay_ms(100);
		
		//ADC记录
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
		//串口数据接收处理
		if (USART_RX_STA & 0x8000)
		{
			LED1=!LED1;
			len = USART_RX_STA & 0x3fff; //得到此次接收到的数据长度
			if (USART_RX_BUF[0] == 0x41) //串口输入了起始符"A",正弦（e.g. A20#B200#!）
			{
				cfr1[0] = 0x00; //RAM 失能
				cfr2[1] = 0x00; //DRG 失能
				Txcfr();		//发送cfrx控制字
				//拆分两组数据
				for (i = 0; USART_RX_BUF[i] != 0x42 && i < len; i++);
				if (i == len) //未找到第2个起始符
				{
					printf("Error!\r\n");
					USART_RX_STA = 0;
					continue;
				}
				ctrl[0]=toint(USART_RX_BUF, i);
				ctrl[1]=toint(USART_RX_BUF + i, len - i);
				Freq_convert(1000*ctrl[0]);
				Amp_convert(ctrl[1]);
				printf("串口控制：正弦信号（%lu kHz / %lu mV）\r\n",ctrl[0],ctrl[1]);
				//LCD_ShowString(60,250,200,16,16,(u8*)"Sine");
				//LCD_ShowxNum(60,250,ctrl[0],8,16,1);
				//LCD_ShowxNum(100,250,ctrl[1],8,16,1);
			}
			else if (USART_RX_BUF[0] == 0x43) //串口输入了起始符"C",方波（e.g. C20#!）
			{
				ctrl[0]=toint(USART_RX_BUF, len);
				Square_wave(1000*ctrl[0]);
				printf("串口控制：方波信号（%lu kHz）\r\n",ctrl[0]);
				//LCD_ShowString(60,250,200,16,16,(u8*)"Square");
				//LCD_ShowxNum(80,250,ctrl[0],8,16,1);
			}
			else if (USART_RX_BUF[0] == 0x44) //串口输入了起始符"D",锯齿波（e.g. D20#!）
			{
				ctrl[0]=toint(USART_RX_BUF, len);
				Sawtooth_wave(1000*ctrl[0]);
				printf("串口控制：锯齿波信号（%lu kHz）\r\n",ctrl[0]);
				//LCD_ShowString(60,250,200,16,16,(u8*)"Sawtooth");
				//LCD_ShowxNum(80,250,ctrl[0],8,16,1);
			}
			else if (USART_RX_BUF[0] == 0x45) //串口输入了起始符"E",扫频（e.g. E100#F100000#G20#H120#!）
			{
				Amp_convert(200);
				//拆分四组数据
				for (i = 0; USART_RX_BUF[i] != 0x46 && i < len; i++);
				if (i == len) //未找到第2个起始符
				{
					printf("Error!\r\n");
					USART_RX_STA = 0;
					continue;
				}
				ctrl[0] = toint(USART_RX_BUF, i);
				for (j = i; USART_RX_BUF[j] != 0x47 && j < len; j++);
				if (j == len) //未找到第3个起始符
				{
					printf("Error!\r\n");
					USART_RX_STA = 0;
					continue;
				}
				ctrl[1] = toint(USART_RX_BUF + i, j - i);
				if (ctrl[0] >= ctrl[1]) //下限高于上限
				{
					printf("Error!\r\n");
					USART_RX_STA = 0;
					continue;
				}
				for (i = j; USART_RX_BUF[i] != 0x48 && i < len; i++);
				if (i == len) //未找到第4个起始符
				{
					printf("Error!\r\n");
					USART_RX_STA = 0;
					continue;
				}
				ctrl[2] = toint(USART_RX_BUF + j, i - j);
				ctrl[3] = toint(USART_RX_BUF + i, len - i);
				SweepFre(ctrl[0], ctrl[1], ctrl[2], ctrl[3] * 1000);
				printf("串口控制：扫频信号（%lu-%lu Hz / %lu Hz / %lu ms）\r\n",ctrl[0],ctrl[1],ctrl[2],ctrl[3]);
				//LCD_ShowString(60,250,200,16,16,(u8*)"Sweep");
			}
			else if (USART_RX_BUF[0] == 0x54) //触发ADC记录
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
		//按键处理
		key = KEY_Scan(0); //得到键值
		if (key)
		{
			LED1=!LED1;
			switch (key)
			{
			case WKUP_PRES:			 //单频正弦
				cfr1[0] = 0x00;		 //RAM 失能
				cfr2[1] = 0x00;		 //DRG 失能
				Txcfr();			 //发送cfrx控制字
				Amp_convert(200);	//写幅度，输入范围：1-650 mV
				Freq_convert(20000); //写频率，输入范围：1-400 000 000Hz
				printf("按键控制：正弦信号\r\n");
				break;
			case KEY0_PRES: //RAM
				if (flag == 0)
				{
					Sawtooth_wave(20000); //锯齿波，采样时间间隔输入范围：4*(1~65536)ns
					printf("按键控制：锯齿波信号\r\n");
					flag = !flag;
				}
				else
				{
					Square_wave(20000); //方波，采样时间间隔输入范围：4*(1~65536)ns
					printf("按键控制：方波信号\r\n");
					flag = !flag;
				}
				break;
			case KEY1_PRES:		  //扫频
				Amp_convert(200); //写幅度，输入范围：1-650 mV
				//扫频波下限频率，上限频率，频率步进（单位：Hz），步进时间间隔（单位：us）
				SweepFre(100, 100000, 20, 120000); //步进时间范围：4*(1~65536)ns
				printf("按键控制：扫频信号\r\n");
				break;
			}
			LED1=!LED1;
		}
	}
}
