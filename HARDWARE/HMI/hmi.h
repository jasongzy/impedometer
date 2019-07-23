#include "delay.h"
#include "sys.h"
#include "usart.h"

void HMISends(char *buf1)		  //字符串发送函数
{
	u8 i=0;
	while(1)
	{
		if(buf1[i]!=0)
		{
			USART_SendData(USART1,buf1[i]);  //发送一个字节
			while(USART_GetFlagStatus(USART1,USART_FLAG_TXE)==RESET){};//等待发送结束
			i++;
		}
		else
			return;
	}
}

void HMISendb(u8 k)		         //字节发送函数
{		 
	u8 i;
	 for(i=0;i<3;i++)
	 {
		if(k!=0)
	 	{
			USART_SendData(USART1,k);  //发送一个字节
			while(USART_GetFlagStatus(USART1,USART_FLAG_TXE)==RESET){};//等待发送结束
		}
		else
			return;
	 }
}

void HMISendstart(void)
{
	delay_ms(200);
	HMISendb(0xff);
	delay_ms(200);
}

void hmi_end(void) //串口屏指令结束
{
	u8 i;
	for(i=0;i<3;i++) 
	{
		delay_ms(5);
		USART_SendData(USART1, 0xff);//向串口1发送数据
		while(USART_GetFlagStatus(USART1,USART_FLAG_TC)!=SET);//等待发送结束
		delay_ms(5);
		//HMISendstart();
	}
}
