#include "sys.h"
#define uchar unsigned char
#define uint unsigned int
#define ulong unsigned long int
#define PI 3.1415926536
#include "math.h"
#include "adc.h"

#include "lcd.h"

float GetMag(void) //输出幅度比
{
	float temp = Get_Adc_Average(ADC_Channel_1,100);
	//float temp = Get_Adc(ADC_Channel_1);
	float volt,volt_dis;
	temp=temp*(3.3/4096.0);
		volt=temp;
		LCD_ShowxNum(130, 130, volt, 1, 16, 0);
		volt_dis = volt - (int)volt;
		volt_dis *= 10000;
		LCD_ShowxNum(172, 130, volt_dis, 4, 16, 0X80);
//  if(volt<=0.9273)
//		return 0.95/10.0+1;
//	else if(volt<=1.02)
//		return 2.1/10.0+1;
//	else if(volt<=1.1601)
//		return 4.7/10.0+1;
//	else if(volt<=1.3132)
//		return 6.8/10.0+1;
//	else if(volt<=1.3841)
//		return 10.0/10.0+1;
//	else if(volt<=1.5968)
//		return 20.0/10.0+1;
//	else if(volt<=1.6089)
//		return 30.0/10.0+1;
//	else if(volt<=1.79)
//		return 47.0/10.0+1;
//	else
//	{
		//Vmag 的输出为 30mV～1.8V 表示 -30dB～+30dB，中间值 0.9V 为 0dB
		temp = 33.333333*(temp)-30.0;
		temp=pow(10,temp/20.0);
		return temp;
//	}
}

float GetPhs(void)//输出相位差
{
	float temp = Get_Adc_Average(ADC_Channel_3,100);
	//float temp = Get_Adc(ADC_Channel_3);
	float volt;
	temp=temp*(3.3/4096.0);
		volt=temp;
		LCD_ShowxNum(130, 150, volt, 1, 16, 0);
		volt = volt - (int)volt;
		volt *= 10000;
		LCD_ShowxNum(172, 150, volt, 4, 16, 0X80);
	//Vphs 的输出为 30mV～1.8V 表示 180 度～0 度，中间值 0.9V 为 90 度
	temp = -98.901099*(temp)+181.978022;
	//temp = -100.0*(temp)+180.0;
	return temp;
}

float GetRe(float Mag,float Phs)//输出阻抗实部
{
	Phs=Phs*2.0*PI/180.0;
	if(Mag*cos(Phs)<0)
		return -Mag*cos(Phs);
	return Mag*cos(Phs);
}

float GetIm(float Mag,float Phs)//输出阻抗虚部
{
	Phs=Phs*2.0*PI/180.0;
	return Mag*sin(Phs);
}
