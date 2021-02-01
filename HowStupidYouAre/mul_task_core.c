#include "mul_task_core.h"

extern DataRestore_typedef *DATABUF[3];
extern ComRestore_typedef  *COMMANDBUF[3];
	/******************************����ı������ڲ����ж�����******************************************/
extern uint8_t Com_Param,Aim_Param,step;
static uint8_t type_buf[256];
static uint8_t OOM_Table[25][3][10];//��˵����Ϊstatic�������ʼֵ����0.

#define Max_Step_Number 9 //ȡ������������ĵ���ά����
/*******************************************************************************************/
/***************************************************************************************
�ж������ĳ�ʼ����
ԭ����OOM_Table�ĵ�һά��type_buf[type]������OOM_Table�ĵڶ�ά��for_me_flag��trans_flag��ɵĶ���������1������
OOM_Table�ĵ���ά�Ǿ�����ж����裬�ǰ��չ�����Ҫ�ƶ��ġ�
****************************************************************************************/
void OOM_Table_init(void)
{
	/*******************************************************************
	command    type: F0    for_me_flag: 1     trans_flag: 0			�����˵ĳ�ʼ��Ϣ����ָ��(����ڶ�ά���±���[trans_flag:for_me_flag]-1)
	*********************************************************************/
			OOM_Table[0][0][0]=BASICINFSET;
			OOM_Table[0][0][1]=THEEND;
	/*******************************************************************
	command    type: F0    for_me_flag: 0     trans_flag: 1			Ҫ����ת���ĳ�ʼ��Ϣ����ָ��
	*********************************************************************/
			OOM_Table[0][1][0]=NoneedDataResponse;
			OOM_Table[0][1][1]=TransDownCombuf;
			OOM_Table[0][1][2]=WaitDataRegist;
			OOM_Table[0][1][3]=THEEND;
			/************���õ��ж�����*�����Ǵ�WaitDataRegist��Ӧ���ж������±��4��ʼ��*******************/
			OOM_Table[0][1][6]=SendErrorReport;
			OOM_Table[0][1][7]=THEEND;
	/*******************************************************************
	command    type: F1    for_me_flag: 1     trans_flag: 0			�������˵Ķ�ȡ����������Ϣָ��
	*********************************************************************/
			OOM_Table[1][0][0]=LoadTerminalBasicInf;
			OOM_Table[1][0][1]=TransUpSendbufOne;
			OOM_Table[1][0][2]=THEEND;
	/*******************************************************************
	command    type: F1    for_me_flag: 0     trans_flag: 1			Ҫ����ת���Ķ�ȡ����������Ϣָ��
	*********************************************************************/
			OOM_Table[1][1][0]=TransDownCombuf;
			OOM_Table[1][1][1]=WaitDataRegist;
			OOM_Table[1][1][2]=THEEND;
			/************���õ��ж�����*�����Ǵ�WaitDataRegist��Ӧ���ж������±��4��ʼ��******************/
			OOM_Table[1][1][5]=SendErrorReport;
			OOM_Table[1][1][6]=THEEND;
	/*******************************************************************
	command    type: F2    for_me_flag: 1     trans_flag: 0			�������˵ĵ�����������ָ��
	*********************************************************************/
			OOM_Table[2][0][0]=ThresHoldSet;
			OOM_Table[2][0][1]=THEEND;
/*******************************************************************
	command    type: F2    for_me_flag: 0     trans_flag: 1			Ҫ����ת���ĵ�����������ָ��
	*********************************************************************/
			OOM_Table[2][1][0]=NoneedDataResponse;
			OOM_Table[2][1][1]=TransDownCombuf;
			OOM_Table[2][1][2]=WaitDataRegist;
			OOM_Table[2][1][3]=THEEND;
			/************���õ��ж�����*�����Ǵ�WaitDataRegist��Ӧ���ж������±��4��ʼ��*******************/
			OOM_Table[2][1][6]=SendErrorReport;
			OOM_Table[2][1][7]=THEEND;
	/*******************************************************************
	command    type: F3    for_me_flag: 1     trans_flag: 0			�������˵ĳ����źŷ���ͨ��ָ��
	*********************************************************************/
			OOM_Table[3][0][0]=GetLaunchAnnouncement;
			OOM_Table[3][0][1]=THEEND;
	/*******************************************************************
	command    type: F4    for_me_flag: 1     trans_flag: 0			�������˵Ķ�ȡflash�ڵĵ���������Ϣָ��
	*********************************************************************/
			OOM_Table[4][0][0]=GetParamAboutLoadFlashInf;
			OOM_Table[4][0][1]=LoadFlashDataAndTransUp;									
			OOM_Table[4][0][2]=THEEND;
	/*******************************************************************
	command    type: F4    for_me_flag: 0     trans_flag: 1			Ҫ����ת���Ķ�ȡflash�ڵĵ���������Ϣָ��
	*********************************************************************/
			
			OOM_Table[4][1][0]=TransDownCombuf;
			OOM_Table[4][1][1]=WaitDataRegist;
			OOM_Table[4][1][2]=THEEND;
			/************���õ��ж�����*�����Ǵ�WaitDataRegist��Ӧ���ж������±��4��ʼ��******************/
			OOM_Table[4][1][5]=SendErrorReport;
			OOM_Table[4][1][6]=THEEND;
			
			
			OOM_Table[4][1][0]=THEEND;//����ָ������������ ���ʺ����ߴ���ת����
	/*******************************************************************
	command    type: F5    for_me_flag: 1     trans_flag: 0			�������˵Ļ�ȡ����������Ϣָ��
	*********************************************************************/
			OOM_Table[5][0][0]=LoadRccInf;
			OOM_Table[5][0][1]=LoadDGStateInf;
			OOM_Table[5][0][2]=LoadStressInf;
			OOM_Table[5][0][3]=LoadTemperatureInf;
			OOM_Table[5][0][4]=LoadSignalAmplitudeInf;
			OOM_Table[5][0][5]=LoadAllTheTrackInf;
			OOM_Table[5][0][6]=TransUpSendbufOne;
			OOM_Table[5][0][7]=THEEND;
	/*******************************************************************
	command    type: F5    for_me_flag: 0     trans_flag: 1			Ҫ����ת���Ļ�ȡ����������Ϣָ��
	*********************************************************************/
			OOM_Table[5][1][0]=TransDownCombuf;
			OOM_Table[5][1][1]=WaitDataRegist;
			OOM_Table[5][1][2]=THEEND;
			/************���õ��ж�����*�����Ǵ�WaitDataRegist��Ӧ���ж������±��4��ʼ��*******************/
			OOM_Table[5][1][5]=SendErrorReport;
			OOM_Table[5][1][6]=THEEND;
	/*******************************************************************
	command    type: F6    for_me_flag: 1     trans_flag: 0			�������˵����������ն˻�����Ϣָ��
	*********************************************************************/
			OOM_Table[6][0][0]=LoadTerminalBasicInf;
			OOM_Table[6][0][1]=TransUpSendbufOne;
			OOM_Table[6][0][2]=THEEND;
/*******************************************************************
	command    type: 52    for_me_flag: 1     trans_flag: 0			�������˵�ʵʱʱ������ָ��ಥ��
	*********************************************************************/
			OOM_Table[8][0][0]=SetRTC;
			OOM_Table[8][0][1]=THEEND;
	/*******************************************************************
	command    type: 52    for_me_flag: 0     trans_flag: 1			Ҫ����ת����ʵʱʱ������ָ��ಥ��
	*********************************************************************/
			OOM_Table[8][1][0]=TransDownCombuf;
			OOM_Table[8][1][1]=RecordMulAddr;
			OOM_Table[8][1][2]=NoneedDataResponse;//��Ϊ����ָ���ʵʱ��������ת����Ȼ��������������
			OOM_Table[8][1][3]=WaitDataRegist;
			OOM_Table[8][1][4]=THEEND;
			/************���õ��ж�����*�����Ǵ�WaitDataRegist��Ӧ���ж������±��4��ʼ��*******************/
			OOM_Table[8][1][7]=SendErrorReport;
			OOM_Table[8][1][8]=THEEND;
	/*******************************************************************
	command    type: 52    for_me_flag: 1     trans_flag: 1			���Ƿ������˵�����Ҫ����ת����ʵʱʱ������ָ��ಥ��
	*********************************************************************/
	
			OOM_Table[8][2][0]=TransDownCombuf;
			OOM_Table[8][2][1]=SetRTC;
			OOM_Table[8][2][2]=RecordMulAddr;
			OOM_Table[8][2][3]=NoneedDataResponse;
			OOM_Table[8][2][4]=WaitDataRegist;
			OOM_Table[8][2][5]=THEEND;
			/************���õ��ж�����*�����Ǵ�WaitDataRegist��Ӧ���ж������±��4��ʼ��*******************/
			OOM_Table[8][2][8]=SendErrorReport;
			OOM_Table[8][2][9]=THEEND;
	/*******************************************************************
	command    type: 55    for_me_flag: 1     trans_flag: 1			���Ƿ������˵�����Ҫ����ת���Ļ�ȡĳ��������Ϣָ��ಥ��
	*********************************************************************/
			OOM_Table[11][2][0]=RecordMulAddr;
			OOM_Table[11][2][1]=TransDownCombuf;
			OOM_Table[11][2][2]=WaitDataRegist;
			OOM_Table[11][2][3]=THEEND;
			/************���õ��ж�����*�����Ǵ�WaitDataRegist��Ӧ���ж������±��4��ʼ��*******************/
			OOM_Table[11][2][6]=LoadDGState;
			OOM_Table[11][2][7]=TransUpSendbufOne;
			OOM_Table[11][2][8]=THEEND;
	/*******************************************************************
	command    type: 55    for_me_flag: 1     trans_flag: 0			�������˵Ļ�ȡĳ��������Ϣָ��ಥ��
	*********************************************************************/
			OOM_Table[11][0][0]=LoadDGState;
			OOM_Table[11][0][1]=TransUpSendbufOne;
			OOM_Table[11][0][2]=THEEND;
	/*******************************************************************
	command    type: 55    for_me_flag: 0     trans_flag: 1			Ҫ����ת���Ļ�ȡĳ��������Ϣָ��ಥ��
	*********************************************************************/
			OOM_Table[11][1][0]=RecordMulAddr;
			OOM_Table[11][1][1]=TransDownCombuf;
			OOM_Table[11][1][2]=WaitDataRegist;
			OOM_Table[11][1][3]=THEEND;
			/************���õ��ж�����*�����Ǵ�WaitDataRegist��Ӧ���ж������±��4��ʼ��*******************/
			OOM_Table[11][1][6]=SendErrorReport;
			OOM_Table[11][1][7]=THEEND;
/*******************************************************************/
/*******************************************************************
	command    type: 56    for_me_flag: 1     trans_flag: 0			�������˵Ĳ���flashָ��ಥ��
	*********************************************************************/
			OOM_Table[9][0][0]=EraseTheFlash;
			OOM_Table[9][0][1]=THEEND;
	/*******************************************************************
	command    type: 56    for_me_flag: 0     trans_flag: 1			Ҫ����ת���Ĳ���flashָ��ಥ��
	*********************************************************************/
			OOM_Table[9][1][0]=TransDownCombuf;
			OOM_Table[9][1][1]=RecordMulAddr;
			OOM_Table[9][1][2]=NoneedDataResponse;
			OOM_Table[9][1][3]=WaitDataRegist;
			OOM_Table[9][1][4]=THEEND;
			/************���õ��ж�����*�����Ǵ�WaitDataRegist��Ӧ���ж������±��4��ʼ��*******************/
			OOM_Table[9][1][7]=SendErrorReport;
			OOM_Table[9][1][8]=THEEND;
	/*******************************************************************
	command    type: 56    for_me_flag: 1     trans_flag: 1			���Ƿ������˵�����Ҫ����ת���Ĳ���flashָ��ಥ��
	*********************************************************************/
	
			OOM_Table[9][2][0]=TransDownCombuf;
			OOM_Table[9][2][1]=EraseTheFlash;
			OOM_Table[9][2][2]=RecordMulAddr;
			OOM_Table[9][2][3]=NoneedDataResponse;
			OOM_Table[9][2][4]=WaitDataRegist;
			OOM_Table[9][2][5]=THEEND;
			/************���õ��ж�����*�����Ǵ�WaitDataRegist��Ӧ���ж������±��4��ʼ��*******************/
			OOM_Table[9][2][8]=SendErrorReport;
			OOM_Table[9][2][9]=THEEND;

/*******************************************************************
	command    type: 57    for_me_flag: 1     trans_flag: 0			�������˵Ĳ���flashָ��ಥ��
	*********************************************************************/
			OOM_Table[12][0][0]=EraseTheBackupRegister;
			OOM_Table[12][0][1]=THEEND;
	/*******************************************************************
	command    type: 57    for_me_flag: 0     trans_flag: 1			Ҫ����ת���Ĳ���flashָ��ಥ��
	*********************************************************************/
			OOM_Table[12][1][0]=TransDownCombuf;
			OOM_Table[12][1][1]=RecordMulAddr;
			OOM_Table[12][1][2]=NoneedDataResponse;
			OOM_Table[12][1][3]=WaitDataRegist;
			OOM_Table[12][1][4]=THEEND;
			/************���õ��ж�����*�����Ǵ�WaitDataRegist��Ӧ���ж������±��4��ʼ��*******************/
			OOM_Table[12][1][7]=SendErrorReport;
			OOM_Table[12][1][8]=THEEND;
	/*******************************************************************
	command    type: 57    for_me_flag: 1     trans_flag: 1			���Ƿ������˵�����Ҫ����ת���Ĳ���flashָ��ಥ��
	*********************************************************************/
	
			OOM_Table[12][2][0]=TransDownCombuf;
			OOM_Table[12][2][1]=EraseTheBackupRegister;
			OOM_Table[12][2][2]=RecordMulAddr;
			OOM_Table[12][2][3]=NoneedDataResponse;
			OOM_Table[12][2][4]=WaitDataRegist;
			OOM_Table[12][2][5]=THEEND;
			/************���õ��ж�����*�����Ǵ�WaitDataRegist��Ӧ���ж������±��4��ʼ��*******************/
			OOM_Table[12][2][8]=SendErrorReport;
			OOM_Table[12][2][9]=THEEND;







/*******************************************************************
	data    type: F1    for_me_flag: 0     trans_flag: 1			
	*********************************************************************/
			OOM_Table[13][1][0]=TransUpTheData;
			OOM_Table[13][1][1]=THEEND;
/*******************************************************************
	data    type: F4    for_me_flag: 0     trans_flag: 1			
	*********************************************************************/
			OOM_Table[16][1][0]=TransUpTheData;
			OOM_Table[16][1][1]=THEEND;
/*******************************************************************
	data    type: F5    for_me_flag: 0     trans_flag: 1			
	*********************************************************************/
			OOM_Table[17][1][0]=TransUpTheData;
			OOM_Table[17][1][1]=THEEND;
	/*******************************************************************
	data    type: 55    for_me_flag: 0     trans_flag: 1			
	*********************************************************************/
			OOM_Table[23][1][0]=TransUpTheData;
			OOM_Table[23][1][1]=THEEND;
	/*******************************************************************
	data    type: 55    for_me_flag: 1     trans_flag: 1			
	*********************************************************************/
			OOM_Table[23][2][0]=AddMyOwnDGstate;
			OOM_Table[23][2][1]=TransUpTheData;
			OOM_Table[23][2][2]=THEEND;
	/*******************************************************************
	data    type: 88    for_me_flag: 0     trans_flag: 1			
	*********************************************************************/
			OOM_Table[22][1][0]=TransUpTheData;
			OOM_Table[22][1][1]=THEEND;

}

/************************************************************************************************
��ָ����Ϊ������±꣬��ָ��������Ԫ��һһ��Ӧ
������������þ��ǰ�ָ����Ƚ�С����ֵһһ��Ӧ���ﵽ��ʡ�ռ��Ŀ��,Ŀǰ������ָ��һһ��Ӧ�ıȽ�С����ֵ���ȡֵΪ12.
***************************************************************************************************/
void type_buf_init(void)
{	
	uint16_t i;
	for (i=0;i<256;i++)
	{
		type_buf[i]=80;//80����㸳��ֵ������������
	}
	
	
	type_buf[0xF0]=0;
	type_buf[0xF1]=1;
	type_buf[0xF2]=2;
	type_buf[0xF3]=3;
	type_buf[0xF4]=4;
	type_buf[0xF5]=5;
	type_buf[0xF6]=6;
//	type_buf[0xF7]=7;
	
	
	type_buf[0x88]=10;
	type_buf[0x52]=8;
	type_buf[0x55]=11;
	type_buf[0x56]=9;
	type_buf[0x57]=12;
}

/*************************************************************************
��ָ�������δ����ȡ�����ȼ���ߵ�ָ������Ϊ��������ֵ����
����ֵ���壺
0����һָ�������δ����ȡ��ָ����Ŀǰ�������ȼ����
1���ڶ�ָ�������δ����ȡ��ָ����Ŀǰ�������ȼ����
2������ָ�������δ����ȡ��ָ����Ŀǰ�������ȼ����
3��Ŀǰָ�������û�пɶ�����
**************************************************************************/
uint8_t Check_ComBuf(ComRestore_typedef  *combuf)
{
	
	if(combuf[0].com_pri==TheFirst && combuf[0].com_used==0) 
				{combuf[0].com_read=1;return 0;}
	else if(combuf[1].com_pri==TheFirst && combuf[1].com_used==0) 
				{combuf[1].com_read=1;return 1;}
	else if(combuf[2].com_pri==TheFirst && combuf[2].com_used==0) 
				{combuf[2].com_read=1;return 2;}
	else if(combuf[0].com_pri==TheSecond && combuf[0].com_used==0) 
				{combuf[0].com_read=1;return 0;}
	else if(combuf[1].com_pri==TheSecond && combuf[1].com_used==0) 
				{combuf[1].com_read=1;return 1;}
	else if(combuf[2].com_pri==TheSecond && combuf[2].com_used==0) 
				{combuf[2].com_read=1;return 2;}
	else if(combuf[0].com_pri==TheLast && combuf[0].com_used ==0) 
				{combuf[0].com_read=1;return 0;}
	else if(combuf[1].com_pri==TheLast && combuf[1].com_used ==0) 
				{combuf[1].com_read=1;return 1;}
	else if(combuf[2].com_pri==TheLast && combuf[2].com_used ==0) 
				{combuf[2].com_read=1;return 2;}
	else 
			{return 3;}
}

uint8_t Check_DataBuf(DataRestore_typedef *databuf,uint8_t waitdattype)
{
	if((databuf[0].datatype== waitdattype || databuf[0].datatype == 0x88 )&& databuf[0].data_used == 0)//�ȴ������ݻ�Ӧ������������waitdattype��Ҳ�������쳣��0x88
		{databuf[0].data_read =1; return 0;}
	else if((databuf[1].datatype== waitdattype  || databuf[1].datatype == 0x88 )&& databuf[1].data_used == 0)
		{databuf[1].data_read =1; return 1;}
	else if((databuf[2].datatype== waitdattype  || databuf[2].datatype == 0x88 )&& databuf[2].data_used == 0)
		{databuf[2].data_read =1; return 2;}
	else
			{return 3;}

}
uint8_t OOM_Init(uint8_t combufnum,uint8_t dataperiod,uint8_t databufnum)
{
	if(dataperiod==1) 
		{
			Com_Param=type_buf[DATABUF[databufnum]->datatype]+12;
			Aim_Param=DATABUF[databufnum]->data_aim-1;
		}
	else
		{
			Com_Param=type_buf[COMMANDBUF[combufnum]->comtype];
			Aim_Param=COMMANDBUF[combufnum]->com_aim-1;
		}
//////	printf(" dataperiod = %d comtype_param= %d,Aim_Param = %d \r\n",dataperiod,Com_Param,Aim_Param);
	if(Com_Param <=24 && Aim_Param <= 2)//24,2ȡ������ά�����ǰ��ά���±귶Χ
	{
		return 1;
	}
	else
	{
		return 0;	
	}
}
/************************************************************************
����ж�����
***********************************************************************/
uint8_t Get_Step(uint8_t comparam,uint8_t aimparam,uint8_t stepnum)
{
	uint8_t taskstep;
	if(stepnum <=Max_Step_Number)
	{
		taskstep=OOM_Table[comparam][aimparam][stepnum];
	}
	else
	{taskstep=100;}//taskstep ����100�Ǵ���ģ�100����һ���ܴ������Ҳ������150,180��
	return taskstep;
}
