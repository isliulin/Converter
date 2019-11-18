/********************************Copyright (c)**********************************\
**                   (c) Copyright 2019, Main, China
**                           All Rights Reserved
**                           By(��ʵ����ҽ�ƿƼ����޹�˾)
**----------------------------------�ļ���Ϣ------------------------------------
** �ļ�����: TrackMission_APP.c
** ������Ա: ����
** ��������: 2019-06-29
** �ĵ�����: �������
******************************************************************************/
#include "myconfig.h"
CAR_CHANGEMISSION_DATA * NOWMISSIONNODE;//��ǰ����ڵ�s
/*ɸѡ����*/
CAR_CHANGEMISSION_DATA * SCREEN_FUNC(void)
{
    OS_CPU_SR cpu_sr;
    CAR_CHANGEMISSION_DATA *TempMissionNode=NULL;//��ʱ��㣬ָ������ת������ͷ���
    CAR_CHANGEMISSION_DATA *ReturnNode=NULL;//���ص�Ԥ��ɸѡ�ڵ�
    CAR_CHANGEMISSION_DATA *MissionNode=NULL;//���ػ�Ľڵ㱸��
	  CAR_CHANGEMISSION_DATA *LastNodeBackUp;//qianjiedian
    TempMissionNode = g_CarApplyChangedata;//ָ��ͷ���
    OS_ENTER_CRITICAL();
    while(TempMissionNode->NextMission!=NULL)
    {
        if(TransStatus.DeviceMode!=OperatingMode)//�˴���ģʽ�����ж�
        {
            if(TempMissionNode->NextMission->Missiontype!=MISSION_AUTO)
            {
                MissionNode=TempMissionNode->NextMission;
                TempMissionNode->NextMission=TempMissionNode->NextMission->NextMission;
                OS_EXIT_CRITICAL();
                return MissionNode;
            }
        }
        else
        {
            if(TempMissionNode->NextMission->MissionMark==MISSION_CARCHAGE||
							TempMissionNode->NextMission->MissionMark==MISSION_FINDZERO||
						TempMissionNode->NextMission->MissionMark==MISSION_LOCATION||
						TempMissionNode->NextMission->MissionMark==MISSION_MOVE)
						//����Ͷ�λ�Լ��˶����������ɸѡ��Ӧ�ö��ǲ����õģ��˴�Ϊ�˷������
            {
                MissionNode=TempMissionNode->NextMission;
                TempMissionNode->NextMission=TempMissionNode->NextMission->NextMission;
                OS_EXIT_CRITICAL();
                return MissionNode;
            }
            if(ISorNotPermisionDo==0&&ReturnNode==NULL&&TempMissionNode->NextMission->MissionMark==MISSION_PROMISSION)
            {
							  LastNodeBackUp=TempMissionNode;
                ReturnNode=TempMissionNode->NextMission;
            }
        }
        TempMissionNode=TempMissionNode->NextMission;
    }
    if(ReturnNode!=NULL)
        LastNodeBackUp->NextMission=ReturnNode->NextMission;
    OS_EXIT_CRITICAL();
    return ReturnNode;
}
/*Ԥ������*/
ErrorType PreAction(u8 Num)
{
    u16 MIN_count=0xffff;//The Min Count;
    u8 chooseTrack;//ѡ��Ĺ����
    u16 D_Value;//��ֵ
    u8 ErgodicTrack;//���������
    chooseTrack=0xff;//Ϊ�˷�ֹ���еĶ��춼������������죬һ�㲻��������������һ������Ǳ궨�����ݶ�ʧ
    MIN_count=0xffff;//��С����ɸѡ����������Ϊ����Ա�������������бȽ�
    /*ѡ�����ŵĶ�����*/
    for(ErgodicTrack=0; ErgodicTrack<MAX_MovingTrack; ErgodicTrack++)
    {
        if(TrackCount[ErgodicTrack][Num]!=0xffff)
        {
            D_Value=abs(TrackCount[ErgodicTrack][Num]-TransStatus.EncoderCount);
            if(D_Value<MIN_count)
            {
                MIN_count=D_Value;
                chooseTrack=ErgodicTrack;
            }
        }
    }
    if(chooseTrack==0xff)//���ѡ�񲻵����죬˵��Ŀ����û�б궨�������ݶ�ʧ���������������û�и���
    {
        TransStatus.TrackUse.Usebit.ExeCommands=T_No;//����ִ�����
        return TrackCountMISSS;
    }
    if(Choose_TransLocate_Change(0,Num)==Num)//����ת�캯��ת�����
    {
        ISorNotPermisionDo=1;//�Ƿ�Ԥ����־����ΪԤ���Ĺ����
        TransStatus.TrackUse.Usebit.ExeCommands=T_No;//����ִ��
        return No_Err;
    }
      return No_Err;
}
/*��λ����*/
ErrorType FixTrack()
{
    OS_CPU_SR cpu_sr;
    CAN_SEND_FRAME *TempResultFram;
    OS_ENTER_CRITICAL();
    TempResultFram=(CAN_SEND_FRAME *)mymalloc(SRAMIN,sizeof(CAN_SEND_FRAME));//���֡
    OS_EXIT_CRITICAL();
    memset(TempResultFram,0,sizeof(CAN_SEND_FRAME));//���뵽���ڴ�����
    Choose_TransLocate_Change(0,NOWMISSIONNODE->InitialPoint);
    BeepChirptimes++;
    if(NOWMISSIONNODE->FarmID!=0x04fe0210)//������ⲿ��λ ��modbus ��Ҫ�ظ�
    {
        /*������ظ�����֡������*/
        TempResultFram->len=1;
        TempResultFram->id=NOWMISSIONNODE->FarmID;//
        TempResultFram->canMsg.dataBuf[0]=0x55;
        TempResultFram->nextMsg=NULL;
        CAN_Post_Queue(CAN2_CHANNEL,TempResultFram);//�뷢�Ͷ���
    }
    OS_ENTER_CRITICAL();
    myfree(SRAMIN,TempResultFram);
    OS_EXIT_CRITICAL();
    TransStatus.TrackUse.Usebit.ExeCommands=T_No;//����ִ������
    return No_Err;
}
/*����*/
ErrorType FindZeroAction()
{
    OS_CPU_SR cpu_sr;
    CAN_SEND_FRAME *TempResultFram;
    OS_ENTER_CRITICAL();
    TempResultFram=(CAN_SEND_FRAME *)mymalloc(SRAMIN,sizeof(CAN_SEND_FRAME));//���֡
    OS_EXIT_CRITICAL();
    memset(TempResultFram,0,sizeof(CAN_SEND_FRAME));//���뵽���ڴ�����
    FindZeroFuction(Need_Back);//���㺯��
    if(NOWMISSIONNODE->FarmID!=0x04fe0211)//������ⲿ��λ ��modbus ��Ҫ�ظ�
    {
        TempResultFram->len=1;
        TempResultFram->id=NOWMISSIONNODE->FarmID;//
        TempResultFram->canMsg.dataBuf[0]=No_Err;
        TempResultFram->nextMsg=NULL;
        CAN_Post_Queue(CAN2_CHANNEL,TempResultFram);//�뷢�Ͷ���
    }
    OS_ENTER_CRITICAL();
    myfree(SRAMIN,TempResultFram);
    OS_EXIT_CRITICAL();
    TransStatus.TrackUse.Usebit.ExeCommands=T_No;//����ִ�������ͷ�
    return No_Err;
}
/*�Լ�*/
ErrorType CheakSelfAction()
{
    OS_CPU_SR cpu_sr;
    CAN_SEND_FRAME *TempResultFram;
    OS_ENTER_CRITICAL();
    TempResultFram=(CAN_SEND_FRAME *)mymalloc(SRAMIN,sizeof(CAN_SEND_FRAME));//���֡
    OS_EXIT_CRITICAL();
    memset(TempResultFram,0,sizeof(CAN_SEND_FRAME));//���뵽���ڴ�����
    u8 MovingErdoc=0;
    u8 StaticErdoc=0;
    u8 chooseErdoc;
    u8 twiceErdoc;
    FindZeroFuction(Need_Back);//������
    for(StaticErdoc=0; StaticErdoc<MAX_FixedTrack; StaticErdoc++) //����count
    {
        if(TrackCount[MovingErdoc][StaticErdoc]!=0xffff)//����0xffff������Ч�Ŀ��Ե����
        {
            chooseErdoc=StaticErdoc;//
        }
    }
    if(TrackCount[0][0]!=0xffff)//����������Ч�ģ��Ͷ�λ�����
    {
        twiceErdoc=0;
    }
    else//������ܶ�λ���Ͷ�λ��һ��
    {
        twiceErdoc=1;
    }
    if(Choose_TransLocate_Change(0,chooseErdoc)==(chooseErdoc|(0x00<<8)))//����TransLocate������ţ�����ı亯�����ҷ���ֵλ��ȷ��	neiyou�ȴ��������ִ���̷߳��ص��ź���
    {
        {
            if(Choose_TransLocate_Change(0,twiceErdoc)==(twiceErdoc|(0x00<<8)))
                /*2019-07-05����:�޸ģ�������֡���Ӵ�����룬������������ݵĵ�һ���ֽ�*/
                //memset(tempNode2->canMsg.dataBuf,0,sizeof(CAN_MSG));
                if(NOWMISSIONNODE->FarmID!=0x04fe0222)//������ⲿ��λ ��modbus ��Ҫ�ظ�                {
                {   TempResultFram->len=8;
                    TempResultFram->id=NOWMISSIONNODE->FarmID;//
                    TempResultFram->canMsg.dataBuf[0]=TransStatus.DeviceMode;//���ֽ��ǹ���ģʽ
                    TempResultFram->canMsg.dataBuf[1]=TransStatus.WarningCode;//�������
                    TempResultFram->canMsg.dataBuf[2]=TransStatus.ErrorCode;//�������
                    TempResultFram->canMsg.dataBuf[3]=TransStatus.DockedNumber;//��׼�Ĺ����//�ɲ�Ҫ
                    TempResultFram->canMsg.dataBuf[4]=TransStatus.TrackUse.TrackStatus;//�����
                    TempResultFram->canMsg.dataBuf[5]=(INT8U)TransStatus.EncoderCount;//�Ͱ�λ//��ǰ������
                    TempResultFram->canMsg.dataBuf[6]=(TransStatus.EncoderCount)>>8;//�߰�λ//��ǰ������
                    TempResultFram->canMsg.dataBuf[7]=TransStatus.TrackUse.TrackStatus;//��ǰλ��//01 12 23 11 22 33 ff
                    TempResultFram->nextMsg=NULL;
                    CAN_Post_Queue(CAN2_CHANNEL,TempResultFram);//�뷢�Ͷ���
                }
            OS_ENTER_CRITICAL();
            myfree(SRAMIN,TempResultFram);
            OS_EXIT_CRITICAL();
            TransStatus.TrackUse.Usebit.ExeCommands=T_No;//����ִ�������ͷ�
        }
    }
    return No_Err;
}
/*�ƶ�*/
ErrorType MoveAction()
{
    INT32S StepMotorCount;
    INT32U MotorCount;
    INT8U dirction;
    INT8U oserr;
    StepMotorCount=0x00000000+(NOWMISSIONNODE->TerminalPoint<<8)+(NOWMISSIONNODE->InitialPoint);
    if((NOWMISSIONNODE->TerminalPoint)>>7==0)
        StepMotorCount=StepMotorCount&0x0000ffff;
    else
        StepMotorCount=StepMotorCount|0xffff0000;
    dirction=(StepMotorCount&0x80000000)>>31;
    if(StepMotorCount<0)
        MotorCount=(~StepMotorCount+1);
    else
        MotorCount=StepMotorCount;
    if(ZeroDirction==anticlockwise)//�������ʱ��У��
    {
        dirction=1-dirction;//����Ϊ����ʱ��dir��0��1��˳ʱ�룬����Ϊ����ʱ��dir��1��0����ʱ��
    }
    else if(ZeroDirction==clockwise)//�����˳ʱ������
    {
        dirction=dirction;//����Ϊ��Ӧ�õ����ʱ��0��ת������Ϊ��Ӧ�õ��˳ָ����ת1
    }
    /*ִ�е���˶�����*/
    StepMotor_Run(100,dirction,MotorCount);
    OSSemPend(arrivePosSem,2000,&oserr);//�ȴ���λ�źţ��ڶ�ʱ��10�жϷ������з���10
    if(oserr==OS_ERR_TIMEOUT)//��ʱ//��Ԥ�ڵ�ʱ�䲻���ϣ�˵�����ܶ���������
    {
        TransStatus.ErrorCode=MotorOverTime;//���������ʱ����
        StepMotor_Run(200,1-dirction,1000);//��ʱ������1000����
        OSSemPend(arrivePosSem,1000,&oserr);//�ȴ���λ�źţ�������ǳ�ʱ��˵���������ߣ��ǳ��Ӱ�ת��������
        if(oserr!=OS_ERR_NONE)////��Ȼ��ʱ
        {
            TransStatus.ErrorCode=BeStuckByCar;//ת������������
        }
        else//��������߳ɹ�
        {
            StepMotor_Run(200,dirction,1000);//��������
        }
        OSSemPend(arrivePosSem,0,&oserr);//�ȴ���λ�źţ��������޵ȴ�������Ȳ�����˵���������ع��ϣ��˹�����
    }
		return No_Err;
}

/*С������*/
DISPATCHSTEP step=T_MOVE_POS0;
ErrorType DispatchCar()
{
    ErrorType err;
    OS_CPU_SR cpu_sr;
    uint8_t sserr;
	  step=T_MOVE_POS0;
    CAN_SEND_FRAME *TempFramup=NULL;//֪ͨС���Ϲ������֡���
    CAN_SEND_FRAME *TempFramdown=NULL;//֪ͨС���¹������֡���
    CAN_SEND_FRAME *CarIsDownTrack=NULL;//֪ͨ�������С�������¹����֡�ڵ㣬���������������·��
    OS_ENTER_CRITICAL();
    TempFramup = (CAN_SEND_FRAME *)mymalloc(SRAMIN,sizeof(CAN_SEND_FRAME));//֪ͨС���Ϲ������֡
    TempFramdown = (CAN_SEND_FRAME *)mymalloc(SRAMIN,sizeof(CAN_SEND_FRAME));//֪ͨС��x�������֡
    CarIsDownTrack = (CAN_SEND_FRAME *)mymalloc(SRAMIN,sizeof(CAN_SEND_FRAME));
    OS_EXIT_CRITICAL();
    int retry=3;
    while(retry>0 && step!=T_FINISH)
    {
        switch(step)
        {
        case T_MOVE_POS0:
        {
            err=Choose_TransLocate_Change(0,NOWMISSIONNODE->InitialPoint);
            if((TrackCount[0][NOWMISSIONNODE->InitialPoint]==TrackCount[0][NOWMISSIONNODE->TerminalPoint])&&
                    (TrackCount[0][NOWMISSIONNODE->InitialPoint]!=0xffff)&&(TrackCount[0][NOWMISSIONNODE->TerminalPoint]!=0xffff))
            {
                step=T_NOTICE_OUT;
            }
            else
            {
                step=T_NOTICE_ENTRY;
            }
        }
        break;
        case T_NOTICE_ENTRY:
        {
            memset(TempFramup,0,sizeof(CAN_SEND_FRAME));//��Ϊ��û����������֡���ݣ�ȫ������Ϊ�㣬��ֹ���ݴ���
            TempFramup->len=2;
            TempFramup->id=NOWMISSIONNODE->CarNum<<16|ThisTransitionNumber<<8|CarCanUpTrack|0x14<<24;//������Ҫ�ĳ�֪ͨС�������Ϲ��������
            TempFramup->canMsg.dataBuf[0]=NOWMISSIONNODE->InitialPoint;//��ʼλ
            TempFramup->canMsg.dataBuf[1]=NOWMISSIONNODE->TerminalPoint;//�յ�λ��
            TempFramup->nextMsg=NULL;
            CAN_Post_Queue(CAN2_CHANNEL,TempFramup);//ѹ������֡����������֪ͨС���Ϲ��֡�����ڷ����߳��еȴ��Է��Ļ�Ӧ��
            step=T_WAIT_ENTRY;
        }
        break;
        case T_WAIT_ENTRY:
        {
            OSSemPend(CarAlreadyUpMbox,0,&sserr);//�ȴ�С���Ѿ��Ϲ��ź�
            step=T_MOVE_POS1;
        }
        break;
        case T_MOVE_POS1:
        {
            Choose_TransLocate_Change(0,NOWMISSIONNODE->TerminalPoint);
            step=T_NOTICE_OUT;
        }
        break;
        case T_NOTICE_OUT:
        {
            memset(TempFramdown,0,sizeof(CAN_SEND_FRAME));
            TempFramdown->len=0;
            TempFramdown->id=NOWMISSIONNODE->CarNum<<16|ThisTransitionNumber<<8|CarCanDownTrack|0x14<<24;//������Ҫ�ĳ�֪ͨС�������¹��������
            //TempFramdown->canMsg.dataBuf[0]=TempMissionNode->InitialPoint;//��ʼλ
            TempFramdown->nextMsg=NULL;
            CAN_Post_Queue(CAN2_CHANNEL,TempFramdown);//ѹ������֡����������֪ͨС���¹��֡�����ڷ����߳��еȴ��Է��Ļ�Ӧ��
            /*С���ո��¹���͸��������������С���Ѿ��¹�״̬֪ͨ֡*/
            //���
            CarIsDownTrack = (CAN_SEND_FRAME *)mymalloc(SRAMIN,sizeof(CAN_SEND_FRAME));//֪ͨ���������С���Ѿ��¹����֡
            memset(CarIsDownTrack,0,sizeof(CAN_SEND_FRAME));//֡���
            CarIsDownTrack->len=1;//������1
            CarIsDownTrack->id=0x04<<24|ThisTransitionNumber<<16|CAN_TRANSFER_MAININDEX<<8|CarIsDowning;
            CarIsDownTrack->canMsg.dataBuf[0]=NOWMISSIONNODE->CarNum;
            CarIsDownTrack->nextMsg=NULL;
            //������
            CAN_Post_Queue(CAN2_CHANNEL,CarIsDownTrack);//�������������
            step=T_WAIT_OUT;
        }
        break;
        case T_WAIT_OUT:
        {
            OSSemPend(CarAlreadyDownSem,15000,&sserr);//�ȴ�С���Ѿ��¹��ź���40S
            ISorNotPermisionDo=0;
            step=T_FINISH;
        }
        break;
        default:
            break;
        }
    }

}
/*ִ�к���*/
ErrorType IMPLEMENT()
{
    ErrorType err=No_Err;
    switch (NOWMISSIONNODE->MissionMark)
    {
    case MISSION_PROMISSION://Ԥ����
        err=PreAction(NOWMISSIONNODE->InitialPoint);
        break;
    case MISSION_CARCHAGE://С������ͨ������ʵ�������С��
        err=DispatchCar();
        break;
    case MISSION_FINDZERO://����
        err=FindZeroAction();
        break;
    case MISSION_CHEKSELF://�Լ��
        err=CheakSelfAction();
        break;
    case MISSION_LOCATION://��λ
        err=FixTrack();
        break;
    case MISSION_MOVE://�ƶ�
        err=MoveAction();
        break;
    }
    return err;
}
/*�ͷź���*/
void RELEASENODE()
{
    OS_CPU_SR cpu_sr;
    OS_ENTER_CRITICAL();
    myfree(SRAMIN,NOWMISSIONNODE);
    OS_EXIT_CRITICAL();
}
/*ɸѡ��ִ������*/
void SCREEN_IMPLEMENT_Task(void *pada)
{
    if(TransStatus.DeviceMode==OperatingMode)//��ʱ���Զ�ģʽ/////////////////////////
        Load_One_FindZero();//�������б������һ������������WΪ�ϵ�����
    while(1)
    {
        if((NOWMISSIONNODE=SCREEN_FUNC())==NULL)
        {
            OSTimeDlyHMSM(0,0,0,10);
            continue;
        }
        if(IMPLEMENT()==No_Err)
        {
            RELEASENODE();
        }
    }
}

/*******************************************************************************
** ��������: TransFindZero_Task
** ��������: �������� ����100
** ����˵��: pdata: [����/��]
** ����˵��: None
** ������Ա: ����
** ��������: 2019-07-03
********************************************************************************/
void TransFindZero_Task(void *pdata)
{
    pdata=pdata;
    while(1)
    {
        //OSSemPend(TransFindzeroSem,0,&oserr);//���޵ȴ������ź���
        if(FindZeroGlob>0)//˵���ж��Ѿ�����
        {
            OSTimeDlyHMSM(0,0,0,10);  //10ms���˲�
            if ((GPIOB->IDR & 0x02) == 0x00)
            {
                FindZeroCtrl=ISOnZero;//��ȡPB1��ֵ�����ʱ�����PB1��0��˵���Ѿ�ѹ��У�㿪��
                FindZeroGlob=0;
            }
            else if((GPIOB->IDR & 0x02) == 0x02)
            {
                FindZeroCtrl=ISAwayZero;//��ȡPB1��ֵ�����ʱ�����PB1��1��˵���Ѿ��뿪У�㿪��
                FindZeroGlob=0;
            }
        }
        else
            OSTimeDlyHMSM(0,0,0,5);  //�ó�CPU

    }
}
/*******************************End of File************************************/




