#include "delay.h"
#include "sys.h"
#include "usart.h"

void HMISends(char *buf1)		  //�ַ������ͺ���
{
	u8 i=0;
	while(1)
	{
		if(buf1[i]!=0)
		{
			USART_SendData(USART1,buf1[i]);  //����һ���ֽ�
			while(USART_GetFlagStatus(USART1,USART_FLAG_TXE)==RESET){};//�ȴ����ͽ���
			i++;
		}
		else
			return;
	}
}

void HMISendb(u8 k)		         //�ֽڷ��ͺ���
{		 
	u8 i;
	 for(i=0;i<3;i++)
	 {
		if(k!=0)
	 	{
			USART_SendData(USART1,k);  //����һ���ֽ�
			while(USART_GetFlagStatus(USART1,USART_FLAG_TXE)==RESET){};//�ȴ����ͽ���
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

void hmi_end(void) //������ָ�����
{
	u8 i;
	for(i=0;i<3;i++) 
	{
		delay_ms(5);
		USART_SendData(USART1, 0xff);//�򴮿�1��������
		while(USART_GetFlagStatus(USART1,USART_FLAG_TC)!=SET);//�ȴ����ͽ���
		delay_ms(5);
		//HMISendstart();
	}
}
