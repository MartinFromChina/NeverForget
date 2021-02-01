#include "mul_task_core.h"

extern DataRestore_typedef *DATABUF[3];
extern ComRestore_typedef  *COMMANDBUF[3];
	/******************************下面的变量用于产生行动步骤******************************************/
extern uint8_t Com_Param,Aim_Param,step;
static uint8_t type_buf[256];
static uint8_t OOM_Table[25][3][10];//据说定义为static的数组初始值都是0.

#define Max_Step_Number 9 //取决于上面数组的第三维长度
/*******************************************************************************************/
/***************************************************************************************
行动步骤表的初始化。
原理是OOM_Table的第一维由type_buf[type]决定，OOM_Table的第二维由for_me_flag，trans_flag组成的二进制数减1决定。
OOM_Table的第三维是具体的行动步骤，是按照工程需要制定的。
****************************************************************************************/
void OOM_Table_init(void)
{
	/*******************************************************************
	command    type: F0    for_me_flag: 1     trans_flag: 0			给本端的初始信息配置指令(数组第二维的下标是[trans_flag:for_me_flag]-1)
	*********************************************************************/
			OOM_Table[0][0][0]=BASICINFSET;
			OOM_Table[0][0][1]=THEEND;
	/*******************************************************************
	command    type: F0    for_me_flag: 0     trans_flag: 1			要本端转发的初始信息配置指令
	*********************************************************************/
			OOM_Table[0][1][0]=NoneedDataResponse;
			OOM_Table[0][1][1]=TransDownCombuf;
			OOM_Table[0][1][2]=WaitDataRegist;
			OOM_Table[0][1][3]=THEEND;
			/************备用的行动步骤*（总是从WaitDataRegist对应的行动步骤下标加4开始）*******************/
			OOM_Table[0][1][6]=SendErrorReport;
			OOM_Table[0][1][7]=THEEND;
	/*******************************************************************
	command    type: F1    for_me_flag: 1     trans_flag: 0			发给本端的读取单点配置信息指令
	*********************************************************************/
			OOM_Table[1][0][0]=LoadTerminalBasicInf;
			OOM_Table[1][0][1]=TransUpSendbufOne;
			OOM_Table[1][0][2]=THEEND;
	/*******************************************************************
	command    type: F1    for_me_flag: 0     trans_flag: 1			要本端转发的读取单点配置信息指令
	*********************************************************************/
			OOM_Table[1][1][0]=TransDownCombuf;
			OOM_Table[1][1][1]=WaitDataRegist;
			OOM_Table[1][1][2]=THEEND;
			/************备用的行动步骤*（总是从WaitDataRegist对应的行动步骤下标加4开始）******************/
			OOM_Table[1][1][5]=SendErrorReport;
			OOM_Table[1][1][6]=THEEND;
	/*******************************************************************
	command    type: F2    for_me_flag: 1     trans_flag: 0			发给本端的单点门限配置指令
	*********************************************************************/
			OOM_Table[2][0][0]=ThresHoldSet;
			OOM_Table[2][0][1]=THEEND;
/*******************************************************************
	command    type: F2    for_me_flag: 0     trans_flag: 1			要本端转发的单点门限配置指令
	*********************************************************************/
			OOM_Table[2][1][0]=NoneedDataResponse;
			OOM_Table[2][1][1]=TransDownCombuf;
			OOM_Table[2][1][2]=WaitDataRegist;
			OOM_Table[2][1][3]=THEEND;
			/************备用的行动步骤*（总是从WaitDataRegist对应的行动步骤下标加4开始）*******************/
			OOM_Table[2][1][6]=SendErrorReport;
			OOM_Table[2][1][7]=THEEND;
	/*******************************************************************
	command    type: F3    for_me_flag: 1     trans_flag: 0			发给本端的超声信号发射通告指令
	*********************************************************************/
			OOM_Table[3][0][0]=GetLaunchAnnouncement;
			OOM_Table[3][0][1]=THEEND;
	/*******************************************************************
	command    type: F4    for_me_flag: 1     trans_flag: 0			发给本端的读取flash内的单点铁轨信息指令
	*********************************************************************/
			OOM_Table[4][0][0]=GetParamAboutLoadFlashInf;
			OOM_Table[4][0][1]=LoadFlashDataAndTransUp;									
			OOM_Table[4][0][2]=THEEND;
	/*******************************************************************
	command    type: F4    for_me_flag: 0     trans_flag: 1			要本端转发的读取flash内的单点铁轨信息指令
	*********************************************************************/
			
			OOM_Table[4][1][0]=TransDownCombuf;
			OOM_Table[4][1][1]=WaitDataRegist;
			OOM_Table[4][1][2]=THEEND;
			/************备用的行动步骤*（总是从WaitDataRegist对应的行动步骤下标加4开始）******************/
			OOM_Table[4][1][5]=SendErrorReport;
			OOM_Table[4][1][6]=THEEND;
			
			
			OOM_Table[4][1][0]=THEEND;//这条指令数据量过大， 不适合无线串口转发。
	/*******************************************************************
	command    type: F5    for_me_flag: 1     trans_flag: 0			发给本端的获取单点铁轨信息指令
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
	command    type: F5    for_me_flag: 0     trans_flag: 1			要本端转发的获取单点铁轨信息指令
	*********************************************************************/
			OOM_Table[5][1][0]=TransDownCombuf;
			OOM_Table[5][1][1]=WaitDataRegist;
			OOM_Table[5][1][2]=THEEND;
			/************备用的行动步骤*（总是从WaitDataRegist对应的行动步骤下标加4开始）*******************/
			OOM_Table[5][1][5]=SendErrorReport;
			OOM_Table[5][1][6]=THEEND;
	/*******************************************************************
	command    type: F6    for_me_flag: 1     trans_flag: 0			发给本端的立即报告终端基础信息指令
	*********************************************************************/
			OOM_Table[6][0][0]=LoadTerminalBasicInf;
			OOM_Table[6][0][1]=TransUpSendbufOne;
			OOM_Table[6][0][2]=THEEND;
/*******************************************************************
	command    type: 52    for_me_flag: 1     trans_flag: 0			发给本端的实时时钟配置指令（多播）
	*********************************************************************/
			OOM_Table[8][0][0]=SetRTC;
			OOM_Table[8][0][1]=THEEND;
	/*******************************************************************
	command    type: 52    for_me_flag: 0     trans_flag: 1			要本端转发的实时时钟配置指令（多播）
	*********************************************************************/
			OOM_Table[8][1][0]=TransDownCombuf;
			OOM_Table[8][1][1]=RecordMulAddr;
			OOM_Table[8][1][2]=NoneedDataResponse;//因为这条指令的实时性所以先转发，然后再做其他事情
			OOM_Table[8][1][3]=WaitDataRegist;
			OOM_Table[8][1][4]=THEEND;
			/************备用的行动步骤*（总是从WaitDataRegist对应的行动步骤下标加4开始）*******************/
			OOM_Table[8][1][7]=SendErrorReport;
			OOM_Table[8][1][8]=THEEND;
	/*******************************************************************
	command    type: 52    for_me_flag: 1     trans_flag: 1			既是发给本端的又是要本端转发的实时时钟配置指令（多播）
	*********************************************************************/
	
			OOM_Table[8][2][0]=TransDownCombuf;
			OOM_Table[8][2][1]=SetRTC;
			OOM_Table[8][2][2]=RecordMulAddr;
			OOM_Table[8][2][3]=NoneedDataResponse;
			OOM_Table[8][2][4]=WaitDataRegist;
			OOM_Table[8][2][5]=THEEND;
			/************备用的行动步骤*（总是从WaitDataRegist对应的行动步骤下标加4开始）*******************/
			OOM_Table[8][2][8]=SendErrorReport;
			OOM_Table[8][2][9]=THEEND;
	/*******************************************************************
	command    type: 55    for_me_flag: 1     trans_flag: 1			既是发给本端的又是要本端转发的获取某段铁轨信息指令（多播）
	*********************************************************************/
			OOM_Table[11][2][0]=RecordMulAddr;
			OOM_Table[11][2][1]=TransDownCombuf;
			OOM_Table[11][2][2]=WaitDataRegist;
			OOM_Table[11][2][3]=THEEND;
			/************备用的行动步骤*（总是从WaitDataRegist对应的行动步骤下标加4开始）*******************/
			OOM_Table[11][2][6]=LoadDGState;
			OOM_Table[11][2][7]=TransUpSendbufOne;
			OOM_Table[11][2][8]=THEEND;
	/*******************************************************************
	command    type: 55    for_me_flag: 1     trans_flag: 0			发给本端的获取某段铁轨信息指令（多播）
	*********************************************************************/
			OOM_Table[11][0][0]=LoadDGState;
			OOM_Table[11][0][1]=TransUpSendbufOne;
			OOM_Table[11][0][2]=THEEND;
	/*******************************************************************
	command    type: 55    for_me_flag: 0     trans_flag: 1			要本端转发的获取某段铁轨信息指令（多播）
	*********************************************************************/
			OOM_Table[11][1][0]=RecordMulAddr;
			OOM_Table[11][1][1]=TransDownCombuf;
			OOM_Table[11][1][2]=WaitDataRegist;
			OOM_Table[11][1][3]=THEEND;
			/************备用的行动步骤*（总是从WaitDataRegist对应的行动步骤下标加4开始）*******************/
			OOM_Table[11][1][6]=SendErrorReport;
			OOM_Table[11][1][7]=THEEND;
/*******************************************************************/
/*******************************************************************
	command    type: 56    for_me_flag: 1     trans_flag: 0			发给本端的擦除flash指令（多播）
	*********************************************************************/
			OOM_Table[9][0][0]=EraseTheFlash;
			OOM_Table[9][0][1]=THEEND;
	/*******************************************************************
	command    type: 56    for_me_flag: 0     trans_flag: 1			要本端转发的擦除flash指令（多播）
	*********************************************************************/
			OOM_Table[9][1][0]=TransDownCombuf;
			OOM_Table[9][1][1]=RecordMulAddr;
			OOM_Table[9][1][2]=NoneedDataResponse;
			OOM_Table[9][1][3]=WaitDataRegist;
			OOM_Table[9][1][4]=THEEND;
			/************备用的行动步骤*（总是从WaitDataRegist对应的行动步骤下标加4开始）*******************/
			OOM_Table[9][1][7]=SendErrorReport;
			OOM_Table[9][1][8]=THEEND;
	/*******************************************************************
	command    type: 56    for_me_flag: 1     trans_flag: 1			既是发给本端的又是要本端转发的擦除flash指令（多播）
	*********************************************************************/
	
			OOM_Table[9][2][0]=TransDownCombuf;
			OOM_Table[9][2][1]=EraseTheFlash;
			OOM_Table[9][2][2]=RecordMulAddr;
			OOM_Table[9][2][3]=NoneedDataResponse;
			OOM_Table[9][2][4]=WaitDataRegist;
			OOM_Table[9][2][5]=THEEND;
			/************备用的行动步骤*（总是从WaitDataRegist对应的行动步骤下标加4开始）*******************/
			OOM_Table[9][2][8]=SendErrorReport;
			OOM_Table[9][2][9]=THEEND;

/*******************************************************************
	command    type: 57    for_me_flag: 1     trans_flag: 0			发给本端的擦除flash指令（多播）
	*********************************************************************/
			OOM_Table[12][0][0]=EraseTheBackupRegister;
			OOM_Table[12][0][1]=THEEND;
	/*******************************************************************
	command    type: 57    for_me_flag: 0     trans_flag: 1			要本端转发的擦除flash指令（多播）
	*********************************************************************/
			OOM_Table[12][1][0]=TransDownCombuf;
			OOM_Table[12][1][1]=RecordMulAddr;
			OOM_Table[12][1][2]=NoneedDataResponse;
			OOM_Table[12][1][3]=WaitDataRegist;
			OOM_Table[12][1][4]=THEEND;
			/************备用的行动步骤*（总是从WaitDataRegist对应的行动步骤下标加4开始）*******************/
			OOM_Table[12][1][7]=SendErrorReport;
			OOM_Table[12][1][8]=THEEND;
	/*******************************************************************
	command    type: 57    for_me_flag: 1     trans_flag: 1			既是发给本端的又是要本端转发的擦除flash指令（多播）
	*********************************************************************/
	
			OOM_Table[12][2][0]=TransDownCombuf;
			OOM_Table[12][2][1]=EraseTheBackupRegister;
			OOM_Table[12][2][2]=RecordMulAddr;
			OOM_Table[12][2][3]=NoneedDataResponse;
			OOM_Table[12][2][4]=WaitDataRegist;
			OOM_Table[12][2][5]=THEEND;
			/************备用的行动步骤*（总是从WaitDataRegist对应的行动步骤下标加4开始）*******************/
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
把指令作为数组的下标，让指令与数组元素一一对应
这个函数的作用就是把指令与比较小的数值一一对应，达到节省空间的目的,目前用于与指令一一对应的比较小的数值最大取值为12.
***************************************************************************************************/
void type_buf_init(void)
{	
	uint16_t i;
	for (i=0;i<256;i++)
	{
		type_buf[i]=80;//80是随便赋的值，无特殊意义
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
把指令缓冲区中未被读取的优先级最高的指令标号最为函数返回值返回
返回值含义：
0，第一指令缓冲区有未被读取的指令且目前它的优先级最高
1，第二指令缓冲区有未被读取的指令且目前它的优先级最高
2，第三指令缓冲区有未被读取的指令且目前它的优先级最高
3，目前指令缓冲区中没有可读数据
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
	if((databuf[0].datatype== waitdattype || databuf[0].datatype == 0x88 )&& databuf[0].data_used == 0)//等待的数据回应可能是正常的waitdattype，也可能是异常的0x88
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
	if(Com_Param <=24 && Aim_Param <= 2)//24,2取决于三维数组的前两维的下标范围
	{
		return 1;
	}
	else
	{
		return 0;	
	}
}
/************************************************************************
获得行动步骤
***********************************************************************/
uint8_t Get_Step(uint8_t comparam,uint8_t aimparam,uint8_t stepnum)
{
	uint8_t taskstep;
	if(stepnum <=Max_Step_Number)
	{
		taskstep=OOM_Table[comparam][aimparam][stepnum];
	}
	else
	{taskstep=100;}//taskstep 等于100是错误的（100代表一个很大的数，也可以用150,180）
	return taskstep;
}
