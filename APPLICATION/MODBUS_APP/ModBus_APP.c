/********************************Copyright (c)**********************************\
**                   (c) Copyright 2019, Main, China
**                           All Rights Reserved
**                           By(达实久信医疗科技有限公司)
**----------------------------------文件信息------------------------------------
** 文件名称: ModBus_APP.c
** 创建人员: 王凯
** 创建日期: 2019-08-01
** 文档描述:
******************************************************************************/
#include "myconfig.h"
OS_EVENT * ModBusFlagSem;
/*更新临时Count位，*/
void ModBus_TempTrackCount_UpData(void)
{
    u8 MovingErdoc=0;
    u8 StaticErdoc=0;
    Reg[10]=0;
    Reg[11]=0;
    Reg[12]=0;
    Reg[13]=0;

    for(MovingErdoc=0; MovingErdoc<MAX_MovingTrack; MovingErdoc++) //遍历count
    {
        for(StaticErdoc=0; StaticErdoc<MAX_FixedTrack; StaticErdoc++) //遍历count
        {
            if(TempTrackCount[MovingErdoc][StaticErdoc]!=0xffff)//不是0xffff就是有效的可以到达的
            {
                Reg[10+MovingErdoc]=Reg[10+MovingErdoc]|(0x0001<<StaticErdoc);//将对应位置一
            }
        }
    }
}
/*更新count标志位，表明哪几个定轨是可以到达的*/
void ModBus_TrackCount_UpData(void)
{
    u8 MovingErdoc=0;
    u8 StaticErdoc=0;
    Reg[14]=0;
    Reg[15]=0;
    Reg[16]=0;
    Reg[17]=0;

    for(MovingErdoc=0; MovingErdoc<MAX_MovingTrack; MovingErdoc++) //遍历count
    {
        for(StaticErdoc=0; StaticErdoc<MAX_FixedTrack; StaticErdoc++) //遍历count
        {
            if(TrackCount[MovingErdoc][StaticErdoc]!=0xffff)//不是0xffff就是有效的可以到达的
            {
                Reg[14+MovingErdoc]=Reg[14+MovingErdoc]|(0x0001<<StaticErdoc);//将对应位置一
            }
        }
    }
}

u16 Get_Mission_Count(void)
{
    CAR_CHANGEMISSION_DATA *TempMissionNode;//临时结点，指向申请转轨任务头结点
    u16 ReturnCount=0;
    TempMissionNode=g_CarApplyChangedata;//临时结点指向头结点
    while(TempMissionNode->NextMission!=NULL)//把新的结点接在链表的最后
    {
        TempMissionNode=TempMissionNode->NextMission;
        ReturnCount++;
    }
    return ReturnCount;
}


/*ModBud处理任务*/
void MODBUS_Task(void *pdata)
{
    INT8U err;
    INT8U Count=0;
    pdata=pdata;
    while(1)
    {
        OSSemPend(ModBusFlagSem,0,&err);
        Reg[ModBus_NowCount]=TransStatus.EncoderCount;//寄存器代表当前编码数
        Reg[ModBus_NowLoction]=TransStatus.DockedNumber;//寄存器代表当前对准的轨道号
        Reg[ModBus_WorkMode]=TransStatus.DeviceMode;//寄存器代表当前设备状态
        Reg[ModBus_WarningCode]=TransStatus.WarningCode;//寄存器代表的值是警告代码
        Reg[ModBus_ErrorCode]=TransStatus.ErrorCode;//寄存器代表的值是错误代码
        Reg[ModBus_ThisTransitionNumber_R]=ThisTransitionNumber;//寄存器代表的值是这本设备的编号
			  Reg[ModBus_MissionCount]=Get_Mission_Count();
			  Reg[ModBus_MemUse] = my_mem_perused(SRAMIN);
        ModBus_TrackCount_UpData();//更新能够到达的位
        Mosbus_Event();//处理MODbus数据，读取或者写入
        ModBusHandleFunc();//ModBus功能
        Count++;
        if(Count==4)
        {
            LED_BlinkTime.LED4_Times++;//灯闪烁，二号灯
            Count=0;
        }
    }
}
/*******************************End of Head************************************/

