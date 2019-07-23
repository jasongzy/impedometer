/**********************************************************
                       康威电子
										 
功能：DAC8552，16位AD，参考源3.0v， 0-65535对应0-3V，双通道自由切换
			显示：12864cog
接口：SYNC-PC9 SCLK-PC10 DIN-PC11 板载接口黑色排 按键接口请参照key.h
时间：2015/11/3
版本：1.0
作者：康威电子
其他：未经作者许可，不得用于其它任何用途

更多电子需求，请到淘宝店，康威电子竭诚为您服务 ^_^
https://shop110336474.taobao.com/?spm=a230r.7195193.1997079397.2.Ic3MRJ

**********************************************************/

#include "task_manage.h"
#include "delay.h"
#include "key.h"
#include "usb_lib.h"
#include "string.h"

#define OUT_KEY  GPIO_ReadInputDataBit(GPIOA,GPIO_Pin_2)//读取按键0
#define FLASH_SAVE_ADDR  0x0801F000  				//设置FLASH 保存地址(必须为偶数)

u8 Firt_In = 1;
u8 Task_Index = 0;
u8 _return=0;
////////////////////////////////////////////////////
/**********************************************************
* 函数功能 ---> USB连接/断开
* 入口参数 ---> usb_sw：USB开启/关闭控制。0：关闭
*                                         1：打开
* 返回数值 ---> none
* 功能说明 ---> none
**********************************************************/	   
void USB_Port_Set(u8 usb_sw)
{
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA, ENABLE);    //使能PORTA时钟
		   	 
	if(usb_sw)	_SetCNTR(_GetCNTR()&(~(1<<1)));//退出断电模式
	else
	{	  
		_SetCNTR(_GetCNTR()|(1<<1));  // 断电模式
		GPIOA->CRH &= 0XFFF00FFF;
		GPIOA->CRH |= 0X00033000;
		PAout(12) = 0;	    		  
	}
}
void USBRelinkConfig(void)
{
	delay_ms(100);
 	USB_Port_Set(0); 	//USB先断开
	delay_ms(600);
	USB_Port_Set(1);	//USB再次连接
	USB_Config();
}
void USB_SendStr(char str[])
{
	USB_TxWrite((uint8_t*)str, strlen(str));
}
///////////////////////////////////////////////////
u8 Task_Delay(u32 delay_time, u8* delay_ID)
{
	static u8 Time_Get = 0;
	static u32 Time_Triger;
    if(Time_Get == 0)
    {
      Time_Triger = SysTimer + delay_time;
      Time_Get = 1;
    }
    if(SysTimer >= Time_Triger)
    { 
      Time_Get = 0;
			*delay_ID = 1;		//	后续代码已被执行一遍
			return 1;
    }
		return 0;
}
void Copybuf2dis(u8 *source, u8 dis[10], u8  dispoint)
{
	dis[0] = *source + 		 '0';
	dis[1] = '.';
	dis[2] = *(source+1) + '0';
	dis[3] = *(source+2) + '0';
	dis[4] = *(source+3) + '0';
	dis[5] = *(source+4) + '0';
	dis[6] = *(source+5) + '0';
	dis[7] = *(source+6) + '0';
	dis[8] = 'V';
	dis[9] = 0;

	if(dispoint < 1) dis[dispoint] += 128;
	//else if(dispoint < 2) dis[dispoint+1] += 128;	
	else dis[dispoint+1] += 128;	
}

void Set_PointFre(u32 Key_Value, u8* Task_ID)
{
	static u8 P_Index = 0;
	static u32 Fre = 0;
	static u8 AdChannel=1;
	
	u16 dac8552_value;
	float fre_buff;
	u8 fre_buf[20];
	u8 display[20];
	u16 time=0;

	if(Firt_In) 
	{
		Key_Value = K_2_L;
		LCD_Show_CEStr(20,0,"  输出电压   ");
		LCD_Show_CEStr(0,6,"↑   ←→     ↓");
		Firt_In = 0;
		_return=1;		
	}
		fre_buf[0] = (u32)Fre%10000000/1000000;
		fre_buf[1] = (u32)Fre%1000000/100000;
		fre_buf[2] = (u32)Fre%100000/10000;
		fre_buf[3] = (u32)Fre%10000/1000;
		fre_buf[4] = (u32)Fre%1000/100;
		fre_buf[5] = (u32)Fre%100/10;
		fre_buf[6] = (u32)Fre%10;
	
	switch(Key_Value)
	{
		case K_4_S: fre_buf[P_Index]++;break;
		case K_4_L: fre_buf[P_Index]++;break;
		case K_5_L: P_Index++;break;
		case K_5_S: P_Index++;break;
		case K_1_L: P_Index--;break;
		case K_1_S: P_Index--;break;
		case K_3_S: fre_buf[P_Index]--;break;
		case K_3_L: fre_buf[P_Index]--;break;
	}
	if(P_Index == 255) P_Index = 6;
	P_Index %= 7;
	if(fre_buf[P_Index] == 255) fre_buf[P_Index] = 9;
	if(fre_buf[P_Index] ==  10) fre_buf[P_Index] = 0;
	//切换通道
	if(Key_Value == K_2_L || Key_Value == K_2_S)
	{
		if(AdChannel>=4) AdChannel = 1;
		if(AdChannel==1) LCD_Show_CEStr(120,0,"1");
		else if (AdChannel==2) LCD_Show_CEStr(120,0,"2");
		else LCD_Show_CEStr(120,0,"3");
		AdChannel++;
	}
	if(Key_Value != K_NO)
	{
		Fre = fre_buf[0]*1000000 + fre_buf[1]*100000 + fre_buf[2]*10000+ fre_buf[3]*1000+
		fre_buf[4]*100 + fre_buf[5]*10 + fre_buf[6];
		if(Fre>3000000) Fre=3000000;
		fre_buf[0] = (u32)Fre%10000000/1000000;
		fre_buf[1] = (u32)Fre%1000000/100000;
		fre_buf[2] = (u32)Fre%100000/10000;
		fre_buf[3] = (u32)Fre%10000/1000;
		fre_buf[4] = (u32)Fre%1000/100;
		fre_buf[5] = (u32)Fre%100/10;
		fre_buf[6] = (u32)Fre%10;
		
		Copybuf2dis(fre_buf, display, P_Index);
//		OLED_ShowString(64-4*6, 2, display);
		OLED_ShowString(0, 2, display);
		//计算电压
		
		dac8552_value = (u16)(((double)Fre*65535)/3000000);
////		if(AdChannel==1) voltage_output(Channel_B, dac8552_value);
////		else if (AdChannel==2) voltage_output(Channel_A, dac8552_value);
////		else 
////		{
////			voltage_output(Channel_B, dac8552_value);
////			voltage_output(Channel_A, dac8552_value);
////		}
		_return=1;
	}
}
