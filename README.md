# 基于 STM32F103 的阻抗测量仪

采用正点原子 F103 精英板作为主控，驱动 DDS (AD9910) 产生可控频率、幅值的正弦信号作为激励，经放大滤波后输入待测阻抗网络和与之串联的参考电阻，使用差分放大电路获得二者的电压，输入 AD8302 幅相检测芯片，输出其幅值比、相位差，再使用单片机板载 ADC 同时对两路信号进行采样和模数转换，最后由单片机进行数据处理与计算。

支持使用外接的触控串口屏与单片机通信来控制 DDS 输出的信号参数，并显示测量结果（模值、相角、实部、虚部）。
