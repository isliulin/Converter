#include "myconfig.h"

/*******************************************************************************
** 函数名称: GetTransStatisticsSend
** 功能描述: 获得统计数据//此函数需要申请内存传递形参为内存指针
** 参数说明: CopyTempNode: [输入/出]
** 返回说明: None
** 创建人员: 王凯
** 创建日期: 2019-07-10
********************************************************************************/
void SendTransStatistics(CAN_DATA_FRAME *tempNode)
{
    CAN_DATA_FRAME *CopyTempNode;
    OS_CPU_SR cpu_sr;
    OS_ENTER_CRITICAL();
    CopyTempNode = (CAN_DATA_FRAME *)mymalloc(SRAMIN,sizeof(CAN_DATA_FRAME));//申请内存
    if(CopyTempNode!=NULL)
    {
        OS_EXIT_CRITICAL();
        memcpy(CopyTempNode,tempNode,sizeof(CAN_DATA_FRAME));//复制帧数据到临时内存中
        memset(CopyTempNode->canMsg.dataBuf,0,sizeof(CAN_MSG));

        memcpy(&(CopyTempNode->canMsg.dataBuf[0]),&TransStatistics.WarningTimes,sizeof(TransStatistics.WarningTimes));
        memcpy(&(CopyTempNode->canMsg.dataBuf[4]),&TransStatistics.ErrTimes,sizeof(TransStatistics.ErrTimes));
        CopyTempNode->dataLen=8;
        CAN2_Single_Send(CopyTempNode);//单帧发送函数

        memcpy(&(CopyTempNode->canMsg.dataBuf[0]),&TransStatistics.CommWarning,sizeof(TransStatistics.CommWarning));
        memcpy(&(CopyTempNode->canMsg.dataBuf[4]),&TransStatistics.CommErr,sizeof(TransStatistics.CommErr));
        CopyTempNode->dataLen=8;
        CAN2_Single_Send(CopyTempNode);//单帧发送函数

        memcpy(&(CopyTempNode->canMsg.dataBuf[0]),&TransStatistics.FindzeroTimes,sizeof(TransStatistics.FindzeroTimes));
        memcpy(&(CopyTempNode->canMsg.dataBuf[4]),&TransStatistics.MoveTimes,sizeof(TransStatistics.MoveTimes));
        CopyTempNode->dataLen=8;
        CAN2_Single_Send(CopyTempNode);//单帧发送函数

        memcpy(&(CopyTempNode->canMsg.dataBuf[0]),&TransStatistics.PassTimes,sizeof(TransStatistics.PassTimes));
        memcpy(&(CopyTempNode->canMsg.dataBuf[4]),&TransStatistics.ShuntTimes,sizeof(TransStatistics.ShuntTimes));
        CopyTempNode->dataLen=8;
        CAN2_Single_Send(CopyTempNode);//单帧发送函数
        OS_ENTER_CRITICAL();
        myfree(SRAMIN,CopyTempNode);
        OS_EXIT_CRITICAL();
    }
    else
    {
        TransStatus.ErrorCode=RAMSpillover;//故障代码显示内存溢出
        TransStatistics.ErrTimes++;
        GPIO_ResetBits(GPIOF,GPIO_Pin_9);//指示灯常亮
    }

}

/*******************************************************************************
** 函数名称: SendTransStatus
** 功能描述: 上传转轨器状态，分为多主模式和主从模式
** 参数说明: CopyTempNode: 要发送的数据帧
** 返回说明: None
** 创建人员: 王凯
** 创建日期: 2019-07-10
********************************************************************************/
void SendTransStatus(CAN_DATA_FRAME *tempNode)
{
    CAN_DATA_FRAME *CopyTempNode;
    OS_CPU_SR cpu_sr;
    OS_ENTER_CRITICAL();
    CopyTempNode = (CAN_DATA_FRAME *)mymalloc(SRAMIN,sizeof(CAN_DATA_FRAME));//申请内存
    if(CopyTempNode!=NULL)
    {
        OS_EXIT_CRITICAL();
        memcpy(CopyTempNode,tempNode,sizeof(CAN_DATA_FRAME));//复制帧数据到临时内存中
        memset(CopyTempNode->canMsg.dataBuf,0,sizeof(CAN_MSG));
        CopyTempNode->dataLen=8;
        CopyTempNode->canMsg.dataBuf[0]=TransStatus.DeviceMode;//最字节是工作模式
        CopyTempNode->canMsg.dataBuf[1]=TransStatus.WarningCode;//警告代码
        CopyTempNode->canMsg.dataBuf[2]=TransStatus.ErrorCode;//错误代码
        CopyTempNode->canMsg.dataBuf[3]=TransStatus.DockedNumber;//对准的轨道号//可不要
        CopyTempNode->canMsg.dataBuf[4]=TransStatus.TrackUse.TrackStatus;//轨道锁
        CopyTempNode->canMsg.dataBuf[5]=(INT8U)TransStatus.EncoderCount;//低八位//当前编码数
        CopyTempNode->canMsg.dataBuf[6]=(TransStatus.EncoderCount)>>8;//高八位//当前编码数
        CopyTempNode->canMsg.dataBuf[7]=TransStatus.TrackUse.TrackStatus;//
        CAN2_Single_Send(CopyTempNode);//单帧发送函数，将填充好的数据发送给2
        OS_ENTER_CRITICAL();
        myfree(SRAMIN,CopyTempNode);//释放内存
        OS_EXIT_CRITICAL();
    }
    else
    {
        TransStatus.ErrorCode=RAMSpillover;//故障代码显示内存溢出
        TransStatistics.ErrTimes++;
        //GPIO_ResetBits(GPIOF,GPIO_Pin_9);//指示灯常亮
    }
}
/*******************************************************************************
** 函数名称: SendTransConfig
** 功能描述: 发送转轨器配置信息函数
** 参数说明: tempNode: [输入/出]
** 返回说明: None
** 创建人员: 王凯
** 创建日期: 2019-07-10
********************************************************************************/
void SendTransConfig(CAN_DATA_FRAME *tempNode)
{
    INT8U GetConfigCount;
    INT8U GetConfigTimes;
    INT8U GetConfigTrackNum;
    CAN_DATA_FRAME *CopyTempNode;
    OS_CPU_SR cpu_sr;
    OS_ENTER_CRITICAL();
    CopyTempNode = (CAN_DATA_FRAME *)mymalloc(SRAMIN,sizeof(CAN_DATA_FRAME));//申请内存
    if(CopyTempNode!=NULL)
    {
        OS_EXIT_CRITICAL();
        memcpy(CopyTempNode,tempNode,sizeof(CAN_DATA_FRAME));//复制帧数据到临时内存中
        memset(CopyTempNode->canMsg.dataBuf,0,sizeof(CAN_MSG));
        CopyTempNode->dataLen=8;
        for(GetConfigTrackNum=0; GetConfigTrackNum<MAX_MovingTrack; GetConfigTrackNum++) //总共轨道数
        {
            for(GetConfigTimes=0; GetConfigTimes<1; GetConfigTimes++)//每个轨道配置需要发送四帧
            {
                for(GetConfigCount=0; GetConfigCount<4; GetConfigCount++)//分四次填满整个can帧的数据位
                {
                    CopyTempNode->canMsg.dataBuf[2*GetConfigCount]=TrackCount[GetConfigTrackNum][GetConfigTimes*4+GetConfigCount];//低八位
                    CopyTempNode->canMsg.dataBuf[2*GetConfigCount+1]=TrackCount[GetConfigTrackNum][GetConfigTimes*4+GetConfigCount]>>8;//高八位
                }
                CAN2_Single_Send(CopyTempNode);//单帧发送函数
            }
        }

        OS_ENTER_CRITICAL();
        myfree(SRAMIN,CopyTempNode);
        OS_EXIT_CRITICAL();
    }
    else
    {
        TransStatus.ErrorCode=RAMSpillover;//故障代码显示内存溢出
        TransStatistics.ErrTimes++;
        GPIO_ResetBits(GPIOF,GPIO_Pin_9);//指示灯常亮
    }
}
/*******************************************************************************
** 函数名称: SendDeviceOnlyID
** 功能描述: 发送设备唯一ARM编号和ID  包括设备CANID
** 参数说明: tempNode: [输入/出]
** 返回说明: None
** 创建人员: 王凯
** 创建日期: 2019-07-17
********************************************************************************/
void SendDeviceOnlyID(CAN_DATA_FRAME *tempNode)
{
    CAN_DATA_FRAME *CopyTempNode;
    OS_CPU_SR cpu_sr;
    OS_ENTER_CRITICAL();
    CopyTempNode = (CAN_DATA_FRAME *)mymalloc(SRAMIN,sizeof(CAN_DATA_FRAME));//申请内存
    if(CopyTempNode!=NULL)
    {
        OS_EXIT_CRITICAL();
        memset(CopyTempNode,0,sizeof(CAN_DATA_FRAME));
        memcpy(CopyTempNode,tempNode,sizeof(CAN_DATA_FRAME));//复制帧数据到临时内存中
        memset(CopyTempNode->canMsg.dataBuf,0,sizeof(CAN_MSG));
        CopyTempNode->dataLen=8;
        CopyTempNode->canMsg.dataBuf[0]=sn0;//ARM标识符最低八位
        CopyTempNode->canMsg.dataBuf[1]=sn0>>8;//9-16
        CopyTempNode->canMsg.dataBuf[2]=sn0>>16;//17-24
        CopyTempNode->canMsg.dataBuf[3]=sn0>>24;//25-32
        CopyTempNode->canMsg.dataBuf[4]=ThisTransitionNumber;//本设备的ID
        CAN2_Single_Send(CopyTempNode);//单帧发送函数//ID in this function to change
        OS_ENTER_CRITICAL();
        myfree(SRAMIN,CopyTempNode);//release the memory
        OS_EXIT_CRITICAL();
    }
    else
    {
        TransStatus.ErrorCode=RAMSpillover;//故障代码显示内存溢出
        TransStatistics.ErrTimes++;
        GPIO_ResetBits(GPIOF,GPIO_Pin_9);//指示灯常亮
    }

}

/*******************************************************************************
** 函数名称: SetCANIDByARMID
** 功能描述: 根据唯一ARM的ID号码，设置CANID，也就是设置本机设备号
** 参数说明: tempNode: [输入/出]
** 返回说明: None
** 创建人员: 王凯
** 创建日期: 2019-07-17
********************************************************************************/
void SetCANIDByARMID(CAN_DATA_FRAME *tempNode)
{
    CAN_DATA_FRAME *CopyTempNode;
    u32 ARMOnlyID;
    OS_CPU_SR cpu_sr;
    OS_ENTER_CRITICAL();
    CopyTempNode = (CAN_DATA_FRAME *)mymalloc(SRAMIN,sizeof(CAN_DATA_FRAME));//申请内存
    if(CopyTempNode!=NULL)
    {
        OS_EXIT_CRITICAL();
        memset(CopyTempNode,0,sizeof(CAN_DATA_FRAME));
        memcpy(CopyTempNode,tempNode,sizeof(CAN_DATA_FRAME));//复制帧数据到临时内存中
        // memset(CopyTempNode->canMsg.dataBuf,0,sizeof(CAN_MSG));
        ARMOnlyID=(CopyTempNode->canMsg.dataBuf[3]<<24)+(CopyTempNode->canMsg.dataBuf[2]<<16)+(CopyTempNode->canMsg.dataBuf[1]<<8)+CopyTempNode->canMsg.dataBuf[0];
        if(ARMOnlyID!=sn0)
        {
            OS_ENTER_CRITICAL();
            myfree(SRAMIN,CopyTempNode);//release the memory
            OS_EXIT_CRITICAL();
            return;
        }
        else
        {
            ThisTransitionNumber=CopyTempNode->canMsg.dataBuf[4];
            OS_ENTER_CRITICAL();
            myfree(SRAMIN,CopyTempNode);//release the memory
            OS_EXIT_CRITICAL();
        }
    }
    else//内存溢出
    {
        TransStatus.ErrorCode=RAMSpillover;//故障代码显示内存溢出
        TransStatistics.ErrTimes++;
        GPIO_ResetBits(GPIOF,GPIO_Pin_9);//指示灯常亮
    }
    return;
}
/*******************************************************************************
** 函数名称: GetCANCommitMode
** 功能描述: 获得CAN通讯模式,上传Can的通讯模式
** 参数说明: tempNode: [输入/出]
** 返回说明: None
** 创建人员: 王凯
** 创建日期: 2019-07-17
********************************************************************************/
void SentCANCommitMode(CAN_DATA_FRAME *tempNode)
{
    CAN_DATA_FRAME *CopyTempNode;
    OS_CPU_SR cpu_sr;
    OS_ENTER_CRITICAL();
    CopyTempNode = (CAN_DATA_FRAME *)mymalloc(SRAMIN,sizeof(CAN_DATA_FRAME));//申请内存
    if(CopyTempNode!=NULL)
    {
        OS_EXIT_CRITICAL();
        // memset(CopyTempNode,0,sizeof(CAN_DATA_FRAME));
        memcpy(CopyTempNode,tempNode,sizeof(CAN_DATA_FRAME));//复制帧数据到临时内存中
        memset(CopyTempNode->canMsg.dataBuf,0,sizeof(CAN_MSG));
        CopyTempNode->dataLen=1;
        CopyTempNode->canMsg.dataBuf[0]=CANCommunicatMode.CanMode.CanModeMask;//将第一个字节填充为Can通讯模式
        CAN2_Single_Send(CopyTempNode);//单帧发送函数//ID in this function to change
        OS_ENTER_CRITICAL();
        myfree(SRAMIN,CopyTempNode);//release the memory
        OS_EXIT_CRITICAL();
    }
}
/*******************************************************************************
** 函数名称: SetCANCommitMode
** 功能描述: 设置Can通讯模式//如果需要返货错误代码，就使用申请的内存，直接将tempNode
数据位的第一位给CAn发送状态。
** 参数说明: tempNode: [输入/出]
** 返回说明: None
** 创建人员: 王凯
** 创建日期: 2019-07-17
********************************************************************************/
void SetCANCommitMode(CAN_DATA_FRAME *tempNode)
{
    CAN_DATA_FRAME *CopyTempNode;
    OS_CPU_SR cpu_sr;
    OS_ENTER_CRITICAL();
    CopyTempNode = (CAN_DATA_FRAME *)mymalloc(SRAMIN,sizeof(CAN_DATA_FRAME));//申请内存
    if(CopyTempNode!=NULL)
    {
        OS_EXIT_CRITICAL();
        memset(CopyTempNode,0,sizeof(CAN_DATA_FRAME));
        memcpy(CopyTempNode,tempNode,sizeof(CAN_DATA_FRAME));//复制帧数据到临时内存中
        memset(CopyTempNode->canMsg.dataBuf,0,sizeof(CAN_MSG));
        CANCommunicatMode.CanMode.CanModeMask=CopyTempNode->canMsg.dataBuf[0];
        /*..............*/
        OS_ENTER_CRITICAL();
        myfree(SRAMIN,CopyTempNode);//release the memory
        OS_EXIT_CRITICAL();
    }


}

/*******************************************************************************
** 函数名称: SentControlConfig
** 功能描述: 获取控制器基本信息//发送控制器基本信息//将控制器基本信息分成三帧发送
** 参数说明: tempNode: [输入/出]
** 返回说明: None
** 创建人员: 王凯
** 创建日期: 2019-07-17
********************************************************************************/
void SentControlConfig(CAN_DATA_FRAME *tempNode)
{
    CAN_DATA_FRAME *CopyTempNode;
    OS_CPU_SR cpu_sr;
    OS_ENTER_CRITICAL();
    CopyTempNode = (CAN_DATA_FRAME *)mymalloc(SRAMIN,sizeof(CAN_DATA_FRAME));//申请内存
    if(CopyTempNode!=NULL)
    {
        OS_EXIT_CRITICAL();
        memset(CopyTempNode,0,sizeof(CAN_DATA_FRAME));//将申请到的内存数据清零
        memcpy(CopyTempNode,tempNode,sizeof(CAN_DATA_FRAME));//复制帧数据到临时内存中
        memset(CopyTempNode->canMsg.dataBuf,0,sizeof(CAN_MSG));//将数据部分清零
        CopyTempNode->canMsg.dataBuf[0]=ControlMessage.ProductSeries;//第一位作为产品系列
        memcpy(&CopyTempNode->canMsg.dataBuf[1],&ControlMessage.HardwareVersion,4);//内存拷贝，将ControlMessage一个子项拷贝到节点
        memset(&CopyTempNode->canMsg.dataBuf[5],0,3);//节点剩下的部分清零
        CopyTempNode->dataLen=8;

        CAN2_Single_Send(CopyTempNode);//单帧发送函数//ID in this function to change//First Frame

        memset(CopyTempNode->canMsg.dataBuf,0,sizeof(CAN_MSG));//将数据部分内存全部清零
        memcpy(&CopyTempNode->canMsg.dataBuf[0],&ControlMessage.SoftwareVersion,8);//内存拷贝，将ControlMessage两个子项拷贝到节点
        CopyTempNode->dataLen=8;
        CAN2_Single_Send(CopyTempNode);//单帧发送函数//ID in this function to change//Secend Fram

        memset(CopyTempNode->canMsg.dataBuf,0,sizeof(CAN_MSG));//数据部分清零
        memcpy(&CopyTempNode->canMsg.dataBuf[0],&ControlMessage.ManufactureData,8);//内存拷贝，将ControlMessage两个子项拷贝到节点
        CopyTempNode->dataLen=8;
        CAN2_Single_Send(CopyTempNode);//单帧发送函数//ID in this function to change//

        OS_ENTER_CRITICAL();
        myfree(SRAMIN,CopyTempNode);//release the memory
        OS_EXIT_CRITICAL();
    }
}

/*******************************************************************************
** 函数名称: UpdataNowLoction
** 功能描述: 更新当前位置信息，如果在1号和二号之间就是12 如果正好对准2号就是22
** 参数说明: None
** 返回说明: None
** 创建人员: 王凯
** 创建日期: 2019-07-10
********************************************************************************/
//void UpdataNowLoction(void)
//{
//    if(TransStatus.EncoderCount==TrackCount[0])
//        TransStatus.NowLoction=0x00;
//    else if(TransStatus.EncoderCount==TrackCount[1])
//        TransStatus.NowLoction=0x11;
//    else if(TransStatus.EncoderCount==TrackCount[2])
//        TransStatus.NowLoction=0x22;
//    else if(TransStatus.EncoderCount==TrackCount[3])
//        TransStatus.NowLoction=0x33;

//    else if(TransStatus.EncoderCount>TrackCount[0]&&TransStatus.EncoderCount<TrackCount[1])
//        TransStatus.NowLoction=0x01;
//    else if(TransStatus.EncoderCount>TrackCount[1]&&TransStatus.EncoderCount<TrackCount[2])
//        TransStatus.NowLoction=0x12;
//    else if(TransStatus.EncoderCount>TrackCount[2]&&TransStatus.EncoderCount<TrackCount[3])
//        TransStatus.NowLoction=0x23;
//    else TransStatus.NowLoction=0xff;
//}


/*******************************************************************************
** 函数名称: ReadMemeryData
** 功能描述: 读取指定内存的数据
** 参数说明: Add: 地址
** 返回说明: None
** 创建人员: 王凯
** 创建日期: 2019-07-17
********************************************************************************/
u32 ReadMemeryData(u32  Add)
{
    static u32 TempDataBuff;
    TempDataBuff=*(vu32 *)Add;
    return TempDataBuff;
}

/*******************************************************************************
** 函数名称: WriteMemeryData
** 功能描述: 往指令内存区域写数据
** 参数说明: Add: [输入/出]
**			 data: [输入/出]
** 返回说明: None
** 创建人员: 王凯
** 创建日期: 2019-07-17
********************************************************************************/
void WriteMemeryData(u32 Add,u32 data)
{
    u32 *TempPoint=(u32 *)Add;
    *TempPoint=data;
    return;
}

/*******************************************************************************
** 函数名称: ModBusHandleFunc
** 功能描述: ModBus寄存器地址改变引起的功能
** 参数说明: : [输入/出]
** 返回说明: None
** 创建人员: 王凯
** 创建日期: 2019-08-01
********************************************************************************/
void ModBusHandleFunc(void)
{
    CAN_DATA_FRAME * tempNode;
    OS_CPU_SR cpu_sr;
    CAR_CHANGEMISSION_DATA *TempMissionNode=NULL;//临时结点，指向申请转轨任务头结点
    CAR_CHANGEMISSION_DATA *DeleteNode=NULL;//分析完成需要删除free的结点
    TempMissionNode =g_CarApplyChangedata;
    INT32S Count;
    CAN_DATA_FRAME *TempNode;
    u8 oserr;
    u32 * mode;//需要邮箱传递的模式变更
    /**********************************伪装移动，移动的参数是编码数，有正负之分，正负代表电机运动的正反******************************/
    if(Reg[ModBus_MoveCount]!=0)//移动编码数
    {
        Count=(INT32S)Reg[ModBus_MoveCount];
        if(Count==31)
        {
            if(ZeroDirction==anticlockwise)//如果是逆时针校零
            {
                StepMotor_Run(20,1,31);//这地方的速度不可太小，否则没有到位信号
            }
            else
            {
                StepMotor_Run(20,0,31);//这地方的速度不可太小，否则没有到位信号
            }
            OSSemPend(arrivePosSem,0,&oserr);
            Reg[ModBus_MoveCount]=0;
        }
        else if(Count==(INT16S)-31)
        {
            if(ZeroDirction==clockwise)//如果是shun时针校零
            {
                StepMotor_Run(20,1,31);//这地方的速度不可太小，否则没有到位信号
            }
            else
            {
                StepMotor_Run(20,0,31);//这地方的速度不可太小，否则没有到位信号
            }
            OSSemPend(arrivePosSem,0,&oserr);
            Reg[ModBus_MoveCount]=0;
        }
        else
        {
            /*将寄存器中的值伪装成CAN帧，将帧Id中的接收设备地址改为非本机设备地址的值，以便区分*/
            OS_ENTER_CRITICAL();
            tempNode=(CAN_DATA_FRAME *)mymalloc(SRAMIN,sizeof(CAN_DATA_FRAME));
            if(tempNode!=NULL)
            {
                OS_EXIT_CRITICAL();
                memset(tempNode,0,sizeof(CAN_DATA_FRAME));
                tempNode->id.canId=0x04fe0212;
                tempNode->canMsg.dataBuf[0]=Count;
                tempNode->canMsg.dataBuf[1]=Count>>8;
                tempNode->nextMsg=NULL;
                Apply_Change_Mission(tempNode);
                Reg[ModBus_MoveCount]=0;
                OS_ENTER_CRITICAL();
                myfree(SRAMIN,tempNode);
                OS_EXIT_CRITICAL();
            }
        }
    }

    /*******************************************伪装定位*******************************************************/
    if(Reg[ModBus_LocNumer]!=0xffff)
    {
        OS_ENTER_CRITICAL();
        tempNode=(CAN_DATA_FRAME *)mymalloc(SRAMIN,sizeof(CAN_DATA_FRAME));
        OS_EXIT_CRITICAL();
        if(tempNode!=NULL)
        {
            memset(tempNode,0,sizeof(CAN_DATA_FRAME));
            tempNode->id.canId=0x04fe0210;
            tempNode->canMsg.dataBuf[0]=Reg[ModBus_LocNumer];
            tempNode->canMsg.dataBuf[1]=Reg[ModBus_LocNumer]>>8;
            tempNode->nextMsg=NULL;
            Apply_Change_Mission(tempNode);
            Reg[ModBus_LocNumer]=0xffff;
            OS_ENTER_CRITICAL();
            myfree(SRAMIN,tempNode);
            OS_EXIT_CRITICAL();
        }
    }
    /*************************************************伪装校零**********************************************************/
    if(Reg[ModBus_FindZero]!=0xffff)
    {
        OS_ENTER_CRITICAL();
        tempNode=(CAN_DATA_FRAME *)mymalloc(SRAMIN,sizeof(CAN_DATA_FRAME));
        if(tempNode!=NULL)
        {
            OS_EXIT_CRITICAL();
            memset(tempNode,0,sizeof(CAN_DATA_FRAME));
            tempNode->id.canId=0x04fe0211;
            tempNode->nextMsg=NULL;
            Apply_Change_Mission(tempNode);
            Reg[ModBus_FindZero]=0xffff;
            OS_ENTER_CRITICAL();
            myfree(SRAMIN,tempNode);
            OS_EXIT_CRITICAL();
        }
    }

    /***************************************伪装停止，停止当前正在执行的任务 多任务的时候需要多次刹车***************************************/
    if(Reg[ModBus_Stop]==0x0000)
    {
        /*预防多次按下停止按键，引起异常*/
        if(TransStatus.TrackUse.Usebit.ExeCommands==T_Yes||IsMotorRun==T_Yes)//只有转轨器正在工作的时候，才会让他停止
        {
            TranferSpeed=TranferStop;//停止标志位
        }
        Reg[ModBus_Stop]=0xffff;
    }
    /**************************************紧急刹车 清空所有的任务并且立刻停止***********************************************/
    if(Reg[ModBus_Stop]==0x0001)
    {
        while(TempMissionNode->NextMission!=NULL)
        {
            DeleteNode=TempMissionNode->NextMission;
            TempMissionNode->NextMission=DeleteNode->NextMission;
            myfree(SRAMIN,DeleteNode);
        }
        TempMissionNode =g_CarApplyChangedata;
        if(TransStatus.TrackUse.Usebit.ExeCommands==T_Yes||IsMotorRun==T_Yes)//只有转轨器正在工作的时候，才会让他停止
        {
            TranferSpeed=TranferStop;//停止标志位
        }
        Reg[ModBus_Stop]=0xffff;
        BeepChirptimes=BeepChirptimes+3;
    }
    /***********************************电机清除故障,无用*******************************************/
    if(Reg[ModBus_Stop]==0x0002)
    {
        /* while(TempMissionNode->NextMission!=NULL)
         {
             DeleteNode=TempMissionNode->NextMission;
             TempMissionNode->NextMission=DeleteNode->NextMission;
             myfree(SRAMIN,DeleteNode);
         }
         TempMissionNode =g_CarApplyChangedata;
         TranferSpeed=TranferStop;//停止标志位*/
        clear_MOTOR_Alarm;
        OSTimeDlyHMSM(0,0,0,100);
        Back_MOTOR_Alarm;
        Reg[ModBus_Stop]=0xffff;//
    }
    /******************************************进入老化测试模式********************************************/
    if(Reg[ModBus_Stop]==0x0003)
    {
        Burnin=1;//1进入老化
        Reg[ModBus_Stop]=0xffff;
    }
    /****************************************停掉所有的BEEP，静音***************************************/
    if(Reg[ModBus_Stop]==0x0004)
    {
        BeepOFFORON=1-BeepOFFORON;
        Reg[ModBus_Stop]=0xffff;

    }
    /******************************************退出老化测试模式****************************************/
    if(Reg[ModBus_Stop]==0x0005)
    {
        Burnin=0;//0
        Reg[ModBus_Stop]=0xffff;
    }



    /***************************************改变模式为手动DebugMode************************************/
    if(Reg[ModBus_Stop]==0x0006)
    {
        /*如果不是自动模式，防止多按；如果不是离线，离线说明can除了问题，没有意义*/
        if(TransStatus.DeviceMode!=DebugMode&&TransStatus.DeviceMode!=OffLine)
        {
            /*当前状态如果是脱机状态，就需要先打开can的时钟,然后发送一帧心跳观察是否CAN线正常*/
            if(TransStatus.DeviceMode==standAlone)
            {
                RCC_APB1PeriphClockCmd(RCC_APB1Periph_CAN2, ENABLE);//开启单线CAN的时钟
                OSTimeDlyHMSM(0,0,0,150);
                /*组包发送心跳帧观测can线是否正常*/
                OS_ENTER_CRITICAL();
                TempNode = (CAN_DATA_FRAME *)mymalloc(SRAMIN,sizeof(CAN_DATA_FRAME));//申请内存
                OS_EXIT_CRITICAL();
//            memset(TempNode->canMsg.dataBuf,0,sizeof(CAN_MSG));
                memset(TempNode,0,sizeof(CAN_DATA_FRAME));
                TempNode->dataLen=0;
                TempNode->id.MasteridBit.Subindex=0x00;//主索引
                TempNode->id.MasteridBit.MainIndex=0x00;//自索引
                TempNode->id.MasteridBit.deviceid=0x00;//设备号//域的地址
                TempNode->id.MasteridBit.ReserveBit=0;//保留位
                TempNode->id.MasteridBit.RetryBit=0;//重发标志
                TempNode->id.MasteridBit.ackBit=0;//应答帧标志
                TempNode->id.MasteridBit.SubframeBit=0;//分帧标志
                TempNode->id.MasteridBit.MasterslaveBit=0;//多主标志
                CAN2_Single_Send(TempNode);//单帧发送函数//这里面如果发送失败，设备状态就会改变为离线咋状态
                OS_ENTER_CRITICAL();
                myfree(SRAMIN,TempNode);
                OS_EXIT_CRITICAL();
            }
            /*如果心跳帧发送成功，或者在转换前设备状态不是脱机状态**/
            if(TransStatus.DeviceMode!=OffLine)//如果是离线的话说明can线出了问题，这个时候就不要再进行变换了
            {
                OS_ENTER_CRITICAL();
                mode=(u32 *)mymalloc(SRAMIN,sizeof(mode));
                OS_EXIT_CRITICAL();
                if(mode!=NULL)
                {
                    *mode=DebugMode+1;//加一是为了防止出现用邮箱传递零值
                    OSMboxPost(ApplyChangeModeBox,mode);
                }
//                OS_ENTER_CRITICAL();
//                myfree(SRAMIN,mode);
//                OS_EXIT_CRITICAL();
            }
            Reg[ModBus_Stop]=0xffff;

        }
        else
        {
            Reg[ModBus_Stop]=0xffff;
        }
    }
/***********************************************改变模式为自动***************************************************************/
    if(Reg[ModBus_Stop]==0x0007)
    {
        /*如果不是自动模式，防止多按；如果不是离线，离线说明can除了问题，没有意义*/
        if(TransStatus.DeviceMode!=OperatingMode&&TransStatus.DeviceMode!=OffLine)
        {
            /*当前状态如果是脱机状态，就需要先打开can的时钟,然后发送一帧心跳观察是否CAN线正常*/
            if(TransStatus.DeviceMode==standAlone)
            {
                RCC_APB1PeriphClockCmd(RCC_APB1Periph_CAN2, ENABLE);//开启单线CAN的时钟
                OSTimeDlyHMSM(0,0,0,150);
                /*组包发送心跳帧观测can线是否正常*/
                OS_ENTER_CRITICAL();
                TempNode = (CAN_DATA_FRAME *)mymalloc(SRAMIN,sizeof(CAN_DATA_FRAME));//申请内存
                OS_EXIT_CRITICAL();
//            memset(TempNode->canMsg.dataBuf,0,sizeof(CAN_MSG));
                memset(TempNode,0,sizeof(CAN_DATA_FRAME));
                TempNode->dataLen=0;
                TempNode->id.MasteridBit.Subindex=0x00;//主索引
                TempNode->id.MasteridBit.MainIndex=0x00;//自索引
                TempNode->id.MasteridBit.deviceid=0x00;//设备号//域的地址
                TempNode->id.MasteridBit.ReserveBit=0;//保留位
                TempNode->id.MasteridBit.RetryBit=0;//重发标志
                TempNode->id.MasteridBit.ackBit=0;//应答帧标志
                TempNode->id.MasteridBit.SubframeBit=0;//分帧标志
                TempNode->id.MasteridBit.MasterslaveBit=0;//多主标志
                CAN2_Single_Send(TempNode);//单帧发送函数//这里面如果发送失败，设备状态就会改变为离线咋状态
                OS_ENTER_CRITICAL();
                myfree(SRAMIN,TempNode);
                OS_EXIT_CRITICAL();
            }
            /*如果心跳帧发送成功，或者在转换前设备状态不是脱机状态*/
            if(TransStatus.DeviceMode!=OffLine)//如果是离线的话说明can线出了问题，这个时候就不要再进行变换了
            {
                OS_ENTER_CRITICAL();
                mode=(u32 *)mymalloc(SRAMIN,sizeof(mode));
                OS_EXIT_CRITICAL();
                if(mode!=NULL)
                {
                    *mode=OperatingMode+1;//加一是为了防止出现用邮箱传递零值
                    OSMboxPost(ApplyChangeModeBox,mode);
                }
//                OS_ENTER_CRITICAL();
//                myfree(SRAMIN,mode);
//                OS_EXIT_CRITICAL();
            }
            Reg[ModBus_Stop]=0xffff;

        }
        else
        {
            Reg[ModBus_Stop]=0xffff;
        }
    }
/**************************************************************改变模式为脱机*************************************************************/
    if(Reg[ModBus_Stop]==0x0008)
    {
        if(TransStatus.DeviceMode!=standAlone)
        {
            if(TransStatus.DeviceMode!=OffLine)//如果当前状态不是离线模式
            {
                OS_ENTER_CRITICAL();
                mode=(u32 *)mymalloc(SRAMIN,sizeof(mode));
                OS_EXIT_CRITICAL();
                if(mode!=NULL)
                {
                    *mode=standAlone+1;//加一是为了防止出现用邮箱传递零值
                    OSMboxPost(ApplyChangeModeBox,mode);
                }
//                OS_ENTER_CRITICAL();
//                myfree(SRAMIN,mode);
//                OS_EXIT_CRITICAL();
            }
            else//如果当前已经是离线模式，就不再需要申请，直接改变
            {
                TransStatus.DeviceMode=standAlone;
            }

            Reg[ModBus_Stop]=0xffff;
        }
        else
        {
            Reg[ModBus_Stop]=0xffff;
        }
    }
    /**********************************************伪装自检帧**********************************************/
    if(Reg[ModBus_CheakSelf]==0x0)
    {
        OS_ENTER_CRITICAL();
        tempNode=(CAN_DATA_FRAME *)mymalloc(SRAMIN,sizeof(CAN_DATA_FRAME));
        if(tempNode!=NULL)
        {
            OS_EXIT_CRITICAL();
            memset(tempNode,0,sizeof(CAN_DATA_FRAME));
            tempNode->id.canId=0x04fe0222;
            tempNode->nextMsg=NULL;
            Apply_Change_Mission(tempNode);

//            memset(tempNode,0,sizeof(CAN_DATA_FRAME));
//            tempNode->id.canId=0x04fe0210;
//            tempNode->canMsg.dataBuf[0]=0;
//            tempNode->canMsg.dataBuf[1]=0;
//            tempNode->nextMsg=NULL;
//            Apply_Change_Mission(tempNode);

            Reg[ModBus_CheakSelf]=0xffff;
            OS_ENTER_CRITICAL();
            myfree(SRAMIN,tempNode);
            OS_EXIT_CRITICAL();
        }


    }
    /**********************************询问告知的校零开关方向0001 校零方向为逆时针*******************************/
    if(Reg[ModBus_CheakSelf]==0x0001)
    {
        Change_diriction(anticlockwise);//校零开关在逆时针方向
        Reg[ModBus_CheakSelf]=0xffff;
    }
    /**********************************询问告知的校零开关方向0002 校零方向为顺时针*******************************/

    if(Reg[ModBus_CheakSelf]==0x0002)
    {
        Change_diriction(clockwise);//校零开关在顺时针方向
        Reg[ModBus_CheakSelf]=0xffff;
    }


    /****************************如果是标定ModBus_Calibration 寄存器前八位为动轨编号，后八位是定轨编号**************************/
    if(Reg[ModBus_Calibration]!=0xffff)
    {
        if(TempTrackCount[(u8)(Reg[ModBus_Calibration]>>8)][(u8)(Reg[ModBus_Calibration])]==0xffff)
        {
            TempTrackCount[(u8)(Reg[ModBus_Calibration]>>8)][(u8)(Reg[ModBus_Calibration])]=TransStatus.EncoderCount;//轨道数的全局变量设置为当前对准的编码数
            TransStatus.DockedNumber=Reg[ModBus_Calibration];//正好对准的轨道号改变
            Calibration_feature((u8)(Reg[ModBus_Calibration]>>8),(u8)(Reg[ModBus_Calibration]));//已经标定的轨道有效标志更新
        }
        else
        {
            TempTrackCount[(u8)(Reg[ModBus_Calibration]>>8)][(u8)(Reg[ModBus_Calibration])]=0xffff;
            Feature_Clean((u8)(Reg[ModBus_Calibration]>>8),(u8)(Reg[ModBus_Calibration]));//清除标定标志位
        }
        Reg[ModBus_Calibration]=0xffff;
        //    Reg[9]=0xffff;
        BeepChirptimes++;
    }
    /****************************************取消本次标定操作，还原为未标定前的数据********************************************************/
    if(Reg[ModBus_SetCalibration]==0x0001)
    {
        memcpy(TempTrackCount,TrackCount,sizeof(TempTrackCount));
        ModBus_TempTrackCount_UpData();//更新已经标定的位
        Reg[ModBus_SetCalibration]=0xffff;
        BeepChirptimes++;

    }
    /********************************************清除所有的标定缓存****************************************************************/
    if(Reg[ModBus_SetCalibration]==0x0002)
    {
        memset(TempTrackCount,0xffff,sizeof(TempTrackCount));
        ModBus_TempTrackCount_UpData();//更新已经标定的位
        Reg[ModBus_SetCalibration]=0xffff;
        BeepChirptimes++;
    }
    /*****************************************标定数据保存到本设备的FLASH*********************************************************/
    if(Reg[ModBus_SetCalibration]==0x0000)
    {
        memcpy(TrackCount,TempTrackCount,sizeof(TrackCount));
        TrackCount_Save();
        Reg[ModBus_SetCalibration]=0xffff;
        BeepChirptimes++;
        ModBus_TempTrackCount_UpData();//更新已经标定的位
        Update_Limit_Count();//更新限位值
    }
    /*********************************************顺指针和逆时针测试************************************************************/
    if(Reg[ModBus_DirictionTest]!=0x0000)
    {
        if(Reg[ModBus_DirictionTest]==0x0001)//顺时针
        {
            StepMotor_Run(200,NCtrl,500);
            OSSemPend(arrivePosSem,1000,&oserr);//等待到位信号，在定时器10中断服务函数中发送
        }
        else if(Reg[ModBus_DirictionTest]==0x0002)
        {
            StepMotor_Run(200,PCtrl,500);
            OSSemPend(arrivePosSem,1000,&oserr);//等待到位信号，在定时器10中断服务函数中发送
        }
        Reg[ModBus_DirictionTest]=0x0000;
    }
    /**********************************************改变设备号23*****************************************************************/
    if(Reg[ModBus_ThisTransitionNumber_W]!=0xffff)
    {
        ThisTransitionNumber=Reg[ModBus_ThisTransitionNumber_W];
        W25QXX_Write((u8*)&ThisTransitionNumber,FLASH_SECTOR_SIZE*2+4,sizeof(ThisTransitionNumber));//将设备号存在FLASH_SECTOR_SIZE*2+4中
        Reg[ModBus_ThisTransitionNumber_W]=0xffff;
        BeepChirptimes++;

    }
}

/*已经标定标志位置函数*/
void Calibration_feature(u8 T,u8 Num)
{
    if(T==0)
    {
        Reg[10]=Reg[10]|(0x0001<<Num);
    }
    else if(T==1)
    {
        Reg[11]=Reg[11]|(0x0001<<Num);
    }
    else if(T==2)
    {
        Reg[12]=Reg[12]|(0x0001<<Num);
    }
    else if(T==3)
    {
        Reg[13]=Reg[13]|(0x0001<<Num);
    }
    else
        return;
    return;
}
/*清除选中的标定位后，将所对应的标志位清零*/
void Feature_Clean(u8 T,u8 Num)
{
    if(T==0)
    {
        Reg[10]=Reg[10]&~(0x0001<<Num);
    }
    else if(T==1)
    {
        Reg[11]=Reg[11]&~(0x0001<<Num);
    }
    else if(T==2)
    {
        Reg[12]=Reg[12]&~(0x0001<<Num);
    }
    else if(T==3)
    {
        Reg[13]=Reg[13]&~(0x0001<<Num);
    }
    else
        return;
    return;
}


/*轨道是否空闲函数*/
bool IS_Track_FREE()
{

    if((TransStatus.DeviceMode==DebugMode)||(g_CarApplyChangedata->NextMission==NULL)||(TransStatus.TrackUse.Usebit.ExeCommands==T_Yes))
    {
        return false;
    }

    return true;
}

/*找出限制编码，最小*/
void Update_Limit_Count(void)
{
    u8 StaticErdoc;//遍历轨道号
    u8 MovingErdoc=0;
    u16 LimitCount;
    LimitCount=0x0;
    for(StaticErdoc=0; StaticErdoc<MAX_FixedTrack; StaticErdoc++) //遍历count
    {
        if(TrackCount[MovingErdoc][StaticErdoc]!=0xffff&&TrackCount[MovingErdoc][StaticErdoc]>LimitCount)//不是0xffff就是有效的可以到达的
        {
            LimitCount=TrackCount[MovingErdoc][StaticErdoc];//有比LimitCount大的数据就更新，最后一次肯定最大
        }
    }
    MAX_Count=LimitCount;//最大编码
    LimitCount=0xffff;
    for(StaticErdoc=0; StaticErdoc<MAX_FixedTrack; StaticErdoc++) //遍历count
    {
        if(TrackCount[MovingErdoc][StaticErdoc]!=0xffff&&TrackCount[MovingErdoc][StaticErdoc]<LimitCount)//不是0xffff就是有效的可以到达的
        {
            LimitCount=TrackCount[MovingErdoc][StaticErdoc];//有比LimitCount大的数据就更新，最后一次肯定最大
        }
    }
    MIN_Count=LimitCount;//
}
/*自运行，把所有能到的轨道都走一遍，走完三遍之后校零*/
void Self_Function(void)//自运行，把所有能到的轨道走一遍
{
    CAN_DATA_FRAME * tempNode;//模拟帧
    OS_CPU_SR cpu_sr;
    u8 MovingErdoc=0;//动轨
    u8 StaticErdoc=0;//定轨
//    u8 TimeCount=0;//自运行轮数
    while(Burnin==1&&TransStatus.ErrorCode==No_Err)//自运行必要条件，电机无故障并且自运行信号有效
    {
        for(StaticErdoc=0; StaticErdoc<MAX_FixedTrack; StaticErdoc++) //遍历count
        {
            if(TrackCount[MovingErdoc][StaticErdoc]!=0xffff)//不是0xffff就是有效的可以到达的
            {
                OS_ENTER_CRITICAL();//进入临界区
                tempNode=(CAN_DATA_FRAME *)mymalloc(SRAMIN,sizeof(CAN_DATA_FRAME));
                if(tempNode!=NULL)
                {
                    OS_EXIT_CRITICAL();//申请成功就退出临界区
                    memset(tempNode,0,sizeof(CAN_DATA_FRAME));
                    tempNode->id.canId=0x04fe0210;//定位
                    tempNode->canMsg.dataBuf[0]=StaticErdoc;
                    tempNode->canMsg.dataBuf[1]=MovingErdoc>>8;
                    tempNode->nextMsg=NULL;
                    Apply_Change_Mission(tempNode);
                    OS_ENTER_CRITICAL();
                    myfree(SRAMIN,tempNode);
                    OS_EXIT_CRITICAL();
                    Reg[19]++;
                }
                // BeepChirptimes++;
                OSTimeDlyHMSM(0,0,0,10000);
            }
            if(TransStatus.ErrorCode!=No_Err)
                break;
        }
//        TimeCount++;//每走完一轮，次数加一
//        if(TimeCount==2)
//        {
//            /*校零*/
//            tempNode=(CAN_DATA_FRAME *)mymalloc(SRAMIN,sizeof(CAN_DATA_FRAME));
//            if(tempNode!=NULL)
//            {
//                memset(tempNode,0,sizeof(CAN_DATA_FRAME));
//                tempNode->id.canId=0x04fe0211;
//                tempNode->nextMsg=NULL;
//                Apply_Change_Mission(tempNode);
//                Reg[ModBus_FindZero]=0xffff;
//                myfree(SRAMIN,tempNode);
//                Reg[19]++;
//            }
//            BeepChirptimes++;
//            TimeCount=0;
//            OSTimeDlyHMSM(0,0,0,8000);
//        }
    }
    //Reg[19]=0;

}

/*在任务队列里面装载一个找零任务,装载位置在队列最前端*/
void Load_One_FindZero(void)
{
    OS_CPU_SR cpu_sr;
    CAR_CHANGEMISSION_DATA *NewMissionNode;//新任务数据结点
    CAR_CHANGEMISSION_DATA *TempMissionNode;//临时结点，指向申请转轨任务头结点
//	  CAR_CHANGEMISSION_DATA *BackUpMissionNode;//备份节点
    OS_ENTER_CRITICAL();//进入临界区
    NewMissionNode = (CAR_CHANGEMISSION_DATA*)mymalloc(SRAMIN,sizeof(CAR_CHANGEMISSION_DATA));//申请内存
    OS_EXIT_CRITICAL();
    if(NewMissionNode!=NULL)
    {
			  NewMissionNode->Missiontype=MISSION_MANUAL;
        NewMissionNode->NextMission=NULL;
        NewMissionNode->MissionMark=3;//是找零任务
        NewMissionNode->FarmID=0x04fe0211;
    }
    NewMissionNode->TimeMark=OSTimeGet();//获得时间标志
    TempMissionNode=g_CarApplyChangedata;//临时结点指向头结点
    TempMissionNode->NextMission=NewMissionNode;//
    NewMissionNode->NextMission=NULL;
}
/************************************************************************************************
*******************找到离零点最近的轨道编号，给找零时候用，找零时候,要回到距离零点最近的轨道号
*******************返回值是选中的轨道号
************************************************************************************************/
unsigned char Find_NearestTrack(void)
{
    u16 MinCount=0xffff;
    u8 TheChooseTrack;
    u8 ErdicCount;
    for(ErdicCount=0; ErdicCount<MAX_FixedTrack; ErdicCount++)
    {
        if(TrackCount[0][ErdicCount]!=0xffff&&TrackCount[0][ErdicCount]<MinCount)
        {
            MinCount=TrackCount[0][ErdicCount];
            TheChooseTrack=ErdicCount;
        }
    }
    return TheChooseTrack;
}

/***********************************************************************************************
** 函数名称: Mode_Change
** 功能描述: 模式改变函数，脱机手动自动之间的相互转换，离线错误状态下可以直接变更为脱机模式
由自动手动脱机之间的相互转换要向域申请，申请--应答--同意；由脱机变为联机需要先打开can然后申请
模式变更。每次模式变更之后，都将任务列表清空。
** 参数说明: mode: [输入/出]
** 返回说明: None
** 创建人员: 王凯
** 创建日期: 2019-09-26
************************************************************************************************/
u8 Mode_Change(u8 mode)
{
    OS_CPU_SR cpu_sr;
    u8 oser;
    CAR_CHANGEMISSION_DATA *TempMissionNode=NULL;//临时结点，指向申请转轨任务头结点
    CAR_CHANGEMISSION_DATA *DeleteNode=NULL;//分析完成需要删除free的结点
    TempMissionNode =g_CarApplyChangedata;
    CAN_SEND_FRAME *ApplyFram=NULL;
    OS_ENTER_CRITICAL();//进入临界区
    ApplyFram = (CAN_SEND_FRAME *)mymalloc(SRAMIN,sizeof(CAN_SEND_FRAME));
    OS_EXIT_CRITICAL();
    memset(ApplyFram,0,sizeof(CAN_SEND_FRAME));
    /*条件不是手动模式是为了防止用户多次按下模式按键；条件不是离线是离线状态下CAN故障不通，此时申请变为联机没有意义*/
    if(mode==DebugMode&&TransStatus.DeviceMode!=DebugMode&&TransStatus.DeviceMode!=OffLine)
    {
        ApplyFram->len=1;
        ApplyFram->id=0x040002fb|(ThisTransitionNumber<<16);
        ApplyFram->canMsg.dataBuf[0]=DebugMode;//变更为手动模式
        ApplyFram->nextMsg=NULL;
        CAN_Post_Queue(CAN2_CHANNEL,ApplyFram);//发送申请的帧
        OSSemPend(AgreeChangeModeSem,0,&oser);//等待域的回应，这个地方的等待应该无限
        if(oser==OS_ERR_NONE)//如果回应
        {
            TransStatus.DeviceMode=DebugMode;//改变模式

            while(TempMissionNode->NextMission!=NULL)//清空任务队列
            {
                DeleteNode=TempMissionNode->NextMission;
                TempMissionNode->NextMission=DeleteNode->NextMission;
                OS_ENTER_CRITICAL();//进入临界区
                myfree(SRAMIN,DeleteNode);
                OS_EXIT_CRITICAL();

            }
            TempMissionNode =g_CarApplyChangedata;//重新指向头结点
            //如果正在执行命令，让其停下
            if(TransStatus.TrackUse.Usebit.ExeCommands==T_Yes)//只有转轨器正在工作的时候，才会让他停止
            {
                TranferSpeed=TranferStop;//停止标志位
            }
        }
    }
		/***********************************************变更为脱机模式**************************************************************/
    else if(mode==standAlone&&TransStatus.DeviceMode!=standAlone)//如果此时不是脱机模式并且要变成脱机模式
    {
        ApplyFram->len=1;
        ApplyFram->id=0x040002fb|(ThisTransitionNumber<<16);
        ApplyFram->canMsg.dataBuf[0]=standAlone;//变更的模式
        ApplyFram->nextMsg=NULL;
        CAN_Post_Queue(CAN2_CHANNEL,ApplyFram);
        OSSemPend(AgreeChangeModeSem,0,&oser);
        if(oser==OS_ERR_NONE)
        {
            TransStatus.DeviceMode=standAlone;
            OSTimeDlyHMSM(0,0,0,150);//让出CPU，状态上传线程发送变换后的状态
            RCC_APB1PeriphClockCmd(RCC_APB1Periph_CAN2, DISABLE);//关闭单线CAN的时钟
            while(TempMissionNode->NextMission!=NULL)
            {
                DeleteNode=TempMissionNode->NextMission;
                TempMissionNode->NextMission=DeleteNode->NextMission;
                OS_ENTER_CRITICAL();//进入临界区
                myfree(SRAMIN,DeleteNode);
                OS_EXIT_CRITICAL();
            }
            TempMissionNode =g_CarApplyChangedata;
            if(TransStatus.TrackUse.Usebit.ExeCommands==T_Yes)//只有转轨器正在工作的时候，才会让他停止
            {
                TranferSpeed=TranferStop;//停止标志位
            }
        }
    }
    /*条件不是自动模式是为了防止用户多次按下模式按键；条件不是离线是离线状态下CAN故障不通，此时申请变为联机没有意义*/
    else if(mode==OperatingMode&&TransStatus.DeviceMode!=OperatingMode&&TransStatus.DeviceMode!=OffLine)//如果此时不是自动模式并且要改变为自动模式
    {
        ApplyFram->len=1;
        ApplyFram->id=0x040002fb|(ThisTransitionNumber<<16);
        ApplyFram->canMsg.dataBuf[0]=OperatingMode;//变更的模式
        ApplyFram->nextMsg=NULL;
        CAN_Post_Queue(CAN2_CHANNEL,ApplyFram);
        OSSemPend(AgreeChangeModeSem,0,&oser);
        if(oser==OS_ERR_NONE)
        {
            TransStatus.DeviceMode=OperatingMode;

            while(TempMissionNode->NextMission!=NULL)
            {
                DeleteNode=TempMissionNode->NextMission;
                TempMissionNode->NextMission=DeleteNode->NextMission;
                OS_ENTER_CRITICAL();//进入临界区
                myfree(SRAMIN,DeleteNode);
                OS_EXIT_CRITICAL();
            }
            TempMissionNode = g_CarApplyChangedata;
            if(TransStatus.TrackUse.Usebit.ExeCommands==T_Yes)//只有转轨器正在工作的时候，才会让他停止
            {
                TranferSpeed=TranferStop;//停止标志位
            }
        }
    }
    OS_ENTER_CRITICAL();//进入临界区
    myfree(SRAMIN,ApplyFram);
    OS_EXIT_CRITICAL();
    return TransStatus.DeviceMode;

}

void SYSYEM_Stable(u8 times)
{
	while(times--)
	{
		BeepON;
		GPIO_ResetBits(GPIOF,GPIO_Pin_9 | GPIO_Pin_10|GPIO_Pin_11);//GPIOF9,F10设置上拉，灯
    GPIO_ResetBits(GPIOG,GPIO_Pin_8 | GPIO_Pin_9);//GPIOG8 G9设置上拉，灯
		delay_ms(100);
	  BeepOFF;
		GPIO_SetBits(GPIOF,GPIO_Pin_9 | GPIO_Pin_10|GPIO_Pin_11);//GPIOF9,F10设置上拉，灯灭
    GPIO_SetBits(GPIOG,GPIO_Pin_8 | GPIO_Pin_9);//GPIOG8 G9设置上拉，灯灭
	  delay_ms(800);
	}
}




