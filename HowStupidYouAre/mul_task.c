#include "mul_task.h"

uint8_t task_sw_flag=0;//任务切换的标志，在系统定时器里，每隔10毫秒置1
uint8_t ComBufNum=0,DatabufNum=0;
uint8_t DataPeriod=0;//该标志置1说明函数Mul_Task处于data触发的任务中


uint8_t Com_Param=0,Aim_Param=0,step_num,task_step;
uint8_t task_complete;

uint8_t SendBufOne[150]={0x66,0xcc},Comnbinedbuf[10];
extern uint8_t SendBufTwo[10];
uint8_t SendBufThree[FlashPackData_Length+20]={0x66,0xcc};//专门用于发送flash中的历史信息
uint16_t  DataBufLength;
/*******************用于等待数据回应的变量***************************/
uint8_t 	wait_data_flag=0,waitdata_type;
uint16_t  wait_data_response_quantity;//等待同种数据回应的数量
extern WaitDataRegist_Typedef *WAITDATABUF[3];
extern uint16_t waitdata_time[3];//waitdata_time_one,waitdata_time_two,waitdata_time_three;
uint8_t waitdata_registnum=0;//该变量等于0代表没有结构体被使用，1代表第一个结构体被使用了，2 ，3依此类推
uint8_t waitData_Trigerednum=0;//该变量表示引发一系列任务的数据注册表的标号
uint8_t NoneedDataResponse_Flag=0;//该变量置一表示不需要等待数据回应（但可能需要等待立即响应）
uint8_t WaitDataAnalysisOver_Flag=0;//
/*******************用于断轨检查的变量***************************/
uint32_t DG_checkcounter=0;
uint8_t DG_StateOne=0,DG_StateOne_faraway=0,DG_StateTwofaraway=0,DG_StateTwo=0,WaveFromFarAway=0;
uint16_t Rec_num[]={0x00},timeout_cnt=0;
uint8_t position_mul;
/**************************************************************/
////////////uint16_t test_addr=1008;
////////////extern uint8_t TrackInfCollect_Buf[1100];

/*******************用于等待立即响应的变量***************************/
extern uint8_t GetImmediaAck_Flag,ImeAck_Direction,ImeAckType;//这两个变量在收到完整立即响应帧但是还没存入缓冲区的时候被赋值
uint8_t ImeAckOk_Flag=0,DataAckOk_Flag=0;
uint16_t WaitDataTime;

/**************************************************************/

/*******************用于存储铁轨信息的数组***************************/
uint8_t RccInfBuf[7];													//,7个元素分别存储：年月日时分秒，其中年占了2个字节，
																							//比如2016年的16进制形式是0x07e0,那么两个字节分别是0x07,0xe0
uint8_t DGStateInfBuf[2];											//
uint8_t StressInfBuf[4]; 											//两根轨的应力，一根两比特
uint8_t TemperatureInfBuf[4];									//两根轨的温度，一根两比特
uint8_t LocalSignalAmplitudeInfBuf[4];				//近端铁轨信号幅值
uint8_t SignalAmplitudeInfBuf[64];						//两根轨的信号幅值，一根轨2个方向，一个方向16比特
/**************************************************************/

uint8_t WLuart_header_addr=0;//用于存储无线串口将要发送的目的终端号
uint8_t Launch_Announcement_Flag=0;//刚刚进行了一次超声信号发射通告的标志。在PFGA配合CPU的情况下，该标志每隔75秒被置一一次


extern uint8_t datarestore_buff[660],comrestore_buff[200];
extern DataRestore_typedef *DATABUF[3];
extern ComRestore_typedef  *COMMANDBUF[3];
extern uint8_t BasicInf_recbuf[6];
extern uint16_t wait_time;
extern uint8_t   MulTrans_startaddr,MulTrans_finaladdr;//收到多播指令后把起点地址和终点地址存入这两个标志里

/***************************控制闪灯的变量****************************/
extern uint8_t Led1_Blink_Times;
extern uint16_t Led1_counter;
extern uint8_t Led1_Freq_Param;

extern uint8_t Led2_Blink_Times;
extern uint16_t Led2_counter;
extern uint8_t Led2_Freq_Param;

extern uint8_t Led3_Blink_Times;
extern uint16_t Led3_counter;
extern uint8_t Led3_Freq_Param;

extern uint8_t Led4_Blink_Times;
extern uint16_t Led4_counter;
extern uint8_t Led4_Freq_Param;
/**********************关于RTC的变量**************************/
extern uint16_t Year;
extern uint8_t Month;
extern uint8_t Day;
extern uint8_t Hour;
extern uint8_t Minute;
extern uint8_t Second;
extern uint32_t Time_Waited_Configured;
/****************************************************************/
/***********************关于应力的变量***********************/
uint8_t Max485_Sendflag=0;//485发送命令的标志
extern uint8_t MAX485_Recbuf[50];
/****************************************************************/
uint8_t tx_flag1,tx_flag2;

TakeAction_typedef  tastate=TA_Idle;

/*************擦除备份寄存器的时候使用该数组*****************/
uint8_t BackupRegEraseSentbuf[20]={
0xFF,0xFF,0xFF,0xFF,0xFF, 
0xFF,0xFF,0xFF,0xFF,0xFF, 
0xFF,0xFF,0xFF,0xFF,0xFF, 
0xFF,0xFF,0xFF,0xFF,0xFF, 
};
/********************************************/
/***************关于上位机读取flash中历史信息的参数******************/
uint32_t Flash_Addr2;
uint16_t Flash_Parabuf[5];
/********************************************/
void Mul_Task(void)
{
	switch(tastate)
		{
/*************************************************************************************************
状态编号：3 
功能：检查任务切换标志task_sw_flag（该变量在系统时钟中断里定期置1），
			如果task_sw_flag=1即代表需要任务切换，进入DataRegistAnaly，状态编号4，否则继续待在TA_Idle状态
****************************************************************************************************/
			case	TA_Idle: 
												#if	defined (USE_MC_DEBUG) && defined (PRINTF_DEBUG)		
													mul_task_debug();
												#endif	
//					if(task_sw_flag ==1)
//						{
//							task_sw_flag=0;
//							tastate=DataRegistAnaly;
//						}
//					else
//						{tastate=TA_Idle;}
								#ifdef  USE_IWDG
										if(task_sw_flag ==1)//这个标志本是用来放慢任务切换速度的，如今用于喂看门狗
											{
												task_sw_flag=0;
												IWDG_Feed();
																#ifdef PRINTF_DEBUG
//																		printf("\r\n IWDG_Feed  \r\n");
																#endif
											}
								#endif
						tastate=DataRegistAnaly;
				break;
/*************************************************************************************************
状态编号：4
功能：检查等待数据标志wait_data_flag（该变量在状态WaitDataRegist中置1，状态编号13）
			如果该标志为0，说明MCU不需要等待接收数据，直接跳转状态TsakSW_Analy，状态编号5
			如果该标志为1，就检查数据接收缓冲区，如果查到了需要的数据就跳转状态TA_OOM_Init，状态编号6
						如果没查到，就跳转状态TsakSW_Analy
****************************************************************************************************/
			case  DataRegistAnaly:
//////											#if	defined (USE_MC_DEBUG) && defined (PRINTF_DEBUG)		
//////													mul_task_debug();
//////												#endif				
					NewWaitData_Analysis();
					
				break;
/*************************************************************************************************
状态编号：5
功能：检查命令缓冲区并且取得最高优先级的命令所对应的缓冲区标号，
			如果没有可执行的命令就跳转状态DGCHECK去执行默认的断轨检查任务
			如果有可执行命令，就把该命令的缓冲区标号记录在全局变量ComBufNum中，并且跳转TA_OOM_Init，状态编号6
****************************************************************************************************/
			case	TsakSW_Analy:
					ComBufNum=Check_ComBuf(COMMANDBUF[0]);
////											#if	defined (USE_MC_DEBUG) && defined (PRINTF_DEBUG)		
////													mul_task_debug();
////												#endif
					if(ComBufNum >=3) {tastate=DGCHECK;}//没有任务切换的时候执行断轨检查任务
					else {tastate=TA_OOM_Init;}
				break;
/*************************************************************************************************
状态编号：1  
功能：执行一次断轨检查任务，然后跳转TA_Idle状态（3）
****************************************************************************************************/
			case	DGCHECK:
//											#if	defined (USE_MC_DEBUG) && defined (PRINTF_DEBUG)		
//													mul_task_debug();
//												#endif
						#ifndef WITHOUT_FPGA
						DG_Check();
						#endif
					tastate=TA_Idle;
					break;
/*************************************************************************************************
状态编号：6
功能：获取行动步骤，具体来说就是：
			根据DataPeriod来判断本次行动步骤是数据触发的还是指令触发的。
			如果DataPeriod=0，就从指令缓冲区中取得指令类型和指令目标组成获取行动步骤的必要参数
			如果DataPeriod=1，就从数据缓冲区中取得数据类型和数据目标组成获取行动步骤的必要参数
			
			如果获取必要参数失败，就跳转TA_Idle状态，获取成功，就跳转TA_TaskBegin，状态编号7
****************************************************************************************************/
			case TA_OOM_Init:
//											#if	defined (USE_MC_DEBUG) && defined (PRINTF_DEBUG)		
//													mul_task_debug();
//												#endif
					if(OOM_Init(ComBufNum,DataPeriod,DatabufNum)==1){tastate=TA_TaskBegin;}
					else {tastate=THEEND;}//万一收到了未定义的指令或数据，要到THEEND状态把该指令或数据的缓冲区置为已使用过
				break;
/*************************************************************************************************
状态编号：7
功能：把行动步骤计数变量step_num清零，然后跳转第0号行动步骤所代表的状态，然后把行动步骤计数变量+1，以便下一次跳转
****************************************************************************************************/
			case TA_TaskBegin:
											#if	defined (USE_MC_DEBUG) && defined (PRINTF_DEBUG)		
													mul_task_debug();
												#endif
					step_num=0;
					GoToNextStep();
//////////					#if	defined (USE_MC_DEBUG) && defined (PRINTF_DEBUG)		
//////////					printf(" dataperiod = %d comtype_param= %d,Aim_Param = %d ,step_num is %d ,step is %d\r\n"
//////////					,DataPeriod,Com_Param,Aim_Param,step_num,tastate);
//////////					#endif
				break;
/*************************************************************************************************
状态编号：2
功能：
****************************************************************************************************/
			case BASICINFSET:
					task_complete= Basic_Inf_Set( COMMANDBUF[ComBufNum]->comcontent);
			
					#ifdef USE_E34
						Write_Parameter(TerminalBasicInf->LocalTerminalAddr);
					#else
						E32_Write_Parameter(TerminalBasicInf->LocalTerminalAddr);
					#endif
					Send_Time_Calculate();
					FPGA_SendWave_Init();
												#if	defined (USE_MC_DEBUG) && defined (PRINTF_DEBUG)		
													mul_task_debug();
												#endif
					/*********************测试flash********************/
				
////////					df_write_open(test_addr);
////////					df_write(TrackInfCollect_Buf,256);
////////					test_addr=test_addr+256;
			    /******************************************************************/
					GoToNextStep();
				break;
/*************************************************************************************************
状态编号：0
功能：一次指令或者数据触发的一系列行动步骤走到了最后一步，清理一些变量和标志。
			如果DataPeriod=1，代表本次行动步骤是数据触发的，就把DataPeriod，wait_data_flag，和数据的data_used标志清除掉�
			如果是指令触发了本轮行动，就把指令所对应的com_used置1.
****************************************************************************************************/
			case THEEND:
											#if	defined (USE_MC_DEBUG) && defined (PRINTF_DEBUG)		
													mul_task_debug();
												#endif
					if(DataPeriod ==1 )
						{
							DataPeriod=0;
							wait_data_flag=0;
							/********************相当于清空接收缓冲区**********************/
								DATABUF[DatabufNum]->data_used=1;
						}
					else
						{
						COMMANDBUF[ComBufNum]->com_used=1;//指令已经被用过了，该缓冲区可以被新收到的指令覆盖了	
						}
					NoneedDataResponse_Flag=0;
					task_complete=0;
					tastate=TA_Idle;
				break;
/*************************************************************************************************
状态编号：8
****************************************************************************************************/
			case LoadTerminalBasicInf:
											#if	defined (USE_MC_DEBUG) && defined (PRINTF_DEBUG)		
													mul_task_debug();
												#endif
					DataBufLength=15;
					task_complete=LoadBasicInf(DataBufLength);
						
					/**********************FLASH调试，N25Q128A*************************/
//									#if	defined (USE_MC_DEBUG) && defined (PRINTF_DEBUG)		
////											printf("\r\n SPI_FLASH_Test \r\n");
////											SPI_FLASH_Test();
////												FLASH_ReadWrite_Test();
//									#endif
					/**********************************************************/
					GoToNextStep();
				break;
/*************************************************************************************************
状态编号：9
****************************************************************************************************/
			case LoadDGState:
											#if	defined (USE_MC_DEBUG) && defined (PRINTF_DEBUG)		
													mul_task_debug();
												#endif
					DataBufLength=12;
					task_complete=LoadDGInf(DataBufLength);
			
					GoToNextStep();
				break;
/*************************************************************************************************
状态编号：16
****************************************************************************************************/
			case RecordMulAddr:
											#if	defined (USE_MC_DEBUG) && defined (PRINTF_DEBUG)		
													mul_task_debug();
											#endif
					MulTrans_startaddr=COMMANDBUF[ComBufNum]->comcontent[6];
					MulTrans_finaladdr=COMMANDBUF[ComBufNum]->com_des_addr;
					GoToNextStep();
				break;
/*************************************************************************************************
状态编号：10
****************************************************************************************************/
			case TransUpSendbufOne:
											#if	defined (USE_MC_DEBUG) && defined (PRINTF_DEBUG)		
													mul_task_debug();
												#endif
						SendBufOne[0]=0x66;SendBufOne[1]=0xcc;
					if(step_num < 8)
						{
							Usart_send(SendBufOne,DataBufLength,COMMANDBUF[ComBufNum]->com_from,1,1);//需要计算校验和，数据装载2位校验和
						}
					else
						{
							Usart_send(SendBufOne,DataBufLength,WAITDATABUF[waitData_Trigerednum]->command_comfrom,1,1);//需要计算校验和，数据装载2位校验和
						}
			
					GoToNextStep();
				break;
/*************************************************************************************************
状态编号：11
****************************************************************************************************/
			case TransDownCombuf:
											#if	defined (USE_MC_DEBUG) && defined (PRINTF_DEBUG)		
													mul_task_debug();
												#endif
					COMMANDBUF[ComBufNum]->comcontent[3]=TerminalBasicInf->LocalTerminalAddr;
					if(COMMANDBUF[ComBufNum]->com_direction == 1) {WLuart_header_addr=TerminalBasicInf->NextTerminalAddr;}
					else if (COMMANDBUF[ComBufNum]->com_direction == 2) {WLuart_header_addr=TerminalBasicInf->FrontTerminalAddr;}
					
					Usart_send(COMMANDBUF[ComBufNum]->comcontent,COMMANDBUF[ComBufNum]->comlength,0,1,0);//
				
					ImeAck_Time_Param_Init(40);//40*50ms = 2s
			
					GoToNextStep();
				break;
/*************************************************************************************************
状态编号：12
一旦走进这个状态，说明指令是配置指令，NoneedDataResponse_Flag要么是1，要么是2。
1代表完全不需要等待数据回应，当指令的目的终端与本端相邻的时候，
					完全不需要等待数据回应，只需等待立即响应即可判断临端是否失联
2代表或许需要等待数据回应。当指令的目的终端非本端（能走到这个状态里，指令的目的终端一定非本端）也非临端的时候，
					等待完立即响应还有可能需要等待失联报告。
****************************************************************************************************/
			case NoneedDataResponse://
						
						wait_data_flag=2;//能走进这个状态里一定需要等待立即响应，一定是配置指令
			
						if(COMMANDBUF[ComBufNum]->com_des_addr == TerminalBasicInf->NextTerminalAddr ||
							COMMANDBUF[ComBufNum]->com_des_addr == TerminalBasicInf->FrontTerminalAddr)
							{NoneedDataResponse_Flag=1;}
						else  
							{NoneedDataResponse_Flag=2;}
										#if	defined (USE_MC_DEBUG) && defined (PRINTF_DEBUG)		
																		mul_task_debug();
													#endif
						GoToNextStep();
				break;
/*************************************************************************************************
状态编号：17
****************************************************************************************************/
			case SendErrorReport:
												#if	defined (USE_MC_DEBUG) && defined (PRINTF_DEBUG)		
													mul_task_debug();
												#endif
							LoadErrorReportInf(10);
							SendBufOne[0]=0x66;SendBufOne[1]=0xcc;
							Usart_send(SendBufOne,10,WAITDATABUF[waitData_Trigerednum]->command_comfrom,1,1);
							GoToNextStep();
				break;
/*************************************************************************************************
状态编号：13
将需要等待的数据回应的类型和方向记录等下来
同时将一旦等到了数据所需的行动步骤参数记录下来
****************************************************************************************************/
			case	WaitDataRegist:
			
						/*******************等待回应的类型只可能是命令类型或者是错误类型0X88*********************************************/
						waitdata_type=COMMANDBUF[ComBufNum]->comtype;
					/***************************************************************************************************/
////////////						if(COMMANDBUF[ComBufNum]->comtype== 0xf4)//获取flash中一段时间内存储的单点铁轨全部信息
////////////							{
////////////								
////////////								wait_data_response_quantity=*48;//flash_read_time_cal*3600/75
////////////							}
////////////						else
							{wait_data_response_quantity=1;}
						/********************如果指令的目的地址是相邻终端，就只需要等待数据回应，否则需要先等待立即响应，再等待数据回应*******/
						/*********************同时需要标记指令的方向，立即响应和数据回应的方向一定与之相反********************************************************************************************/
						if(NoneedDataResponse_Flag == 0)//指令是读取信息指令（如果是配置指令，NoneedDataResponse_Flag=1或者NoneedDataResponse_Flag=2）
							{
								if(COMMANDBUF[ComBufNum]->com_des_addr == TerminalBasicInf->NextTerminalAddr ||
									COMMANDBUF[ComBufNum]->com_des_addr == TerminalBasicInf->FrontTerminalAddr)
									{wait_data_flag=1;}
								else  
									{
										wait_data_flag=2;
										NoneedDataResponse_Flag=3;
									}
									
							}
																#if	defined (USE_MC_DEBUG) && defined (PRINTF_DEBUG)		
																	mul_task_debug();
																#endif
						WaitData_Regist(COMMANDBUF[ComBufNum]->com_direction,waitdata_type,step_num-1,wait_data_flag,WaitDataTime,COMMANDBUF[ComBufNum]->com_from,NoneedDataResponse_Flag,wait_data_response_quantity);
						GoToNextStep();
				break;
/*************************************************************************************************
状态编号：14
****************************************************************************************************/
	  	case TransUpTheData:
											#if	defined (USE_MC_DEBUG) && defined (PRINTF_DEBUG)		
													mul_task_debug();
												#endif
			/********************************改变数据原地址*************************************/
						DATABUF[DatabufNum]->datacontent[4]=TerminalBasicInf->LocalTerminalAddr;
			/********************************改变数据的目的地址************************************************************/
						if(WAITDATABUF[waitData_Trigerednum]->command_comfrom == 1) {DATABUF[DatabufNum]->datacontent[5]=0xff;}
						else if(DATABUF[DatabufNum]->data_direction == 1) //
							{DATABUF[DatabufNum]->datacontent[5]=TerminalBasicInf->NextTerminalAddr;}
						else if(DATABUF[DatabufNum]->data_direction == 2) 
							{DATABUF[DatabufNum]->datacontent[5]=TerminalBasicInf->FrontTerminalAddr;}
				/******************************发送数据**************************************************************/			
						Usart_send(DATABUF[DatabufNum]->datacontent,DATABUF[DatabufNum]->datalength,WAITDATABUF[waitData_Trigerednum]->command_comfrom ,1,1);
						GoToNextStep();
	  		break;
/*************************************************************************************************
状态编号：15
****************************************************************************************************/
			case AddMyOwnDGstate:
											#if	defined (USE_MC_DEBUG) && defined (PRINTF_DEBUG)		
													mul_task_debug();
												#endif
			
							Comnbinedbuf[0]=TerminalBasicInf->LocalTerminalAddr;
							Comnbinedbuf[1]=DG_StateOne;
							Comnbinedbuf[2]=DG_StateTwo;
							DataContentComnbined(DATABUF[DatabufNum]->datacontent,DATABUF[DatabufNum]->datalength,3,Comnbinedbuf);
							
							DATABUF[DatabufNum]->datalength=(DATABUF[DatabufNum]->datalength)+3;
							GoToNextStep();
				break;
/*************************************************************************************************
状态编号：18
****************************************************************************************************/
			case LoadRccInf:				//18
											#if	defined (USE_MC_DEBUG) && defined (PRINTF_DEBUG)		
													mul_task_debug();
												#endif
//							Seconds_to_Date();
//							LoadRcc_Inf();							
							GoToNextStep();
				break;
/*************************************************************************************************
状态编号：19
****************************************************************************************************/
			case LoadDGStateInf:		//19
											#if	defined (USE_MC_DEBUG) && defined (PRINTF_DEBUG)		
													mul_task_debug();
												#endif
//							LoadDGState_inf();
							GoToNextStep();
				break;
/*************************************************************************************************
状态编号：20
****************************************************************************************************/
			case LoadStressInf:		//20
											#if	defined (USE_MC_DEBUG) && defined (PRINTF_DEBUG)		
													mul_task_debug();
												#endif
					GoToNextStep();

							
				break;
/*************************************************************************************************
状态编号：21
****************************************************************************************************/
			case LoadTemperatureInf:			//21
				
							LoadTemperature_Inf();
			
											#if	defined (USE_MC_DEBUG) && defined (PRINTF_DEBUG)		
													mul_task_debug();
												#endif
							
							GoToNextStep();
				break;
/*************************************************************************************************
状态编号：22
****************************************************************************************************/
			case LoadSignalAmplitudeInf://22
											#if	defined (USE_MC_DEBUG) && defined (PRINTF_DEBUG)		
													mul_task_debug();
												#endif
//							LoadSignalAmplitude_Inf();
							GoToNextStep();
				break;
/*************************************************************************************************
状态编号：23
****************************************************************************************************/
			case LoadAllTheTrackInf:		//23
											#if	defined (USE_MC_DEBUG) && defined (PRINTF_DEBUG)		
													mul_task_debug();
												#endif
							DataBufLength=94;
							task_complete=LoadAllTheTrack_Inf(DataBufLength);
							GoToNextStep();
				break;

/*************************************************************************************************
状态编号：24
****************************************************************************************************/
			case EraseTheFlash:		//24
											#if	defined (USE_MC_DEBUG) && defined (PRINTF_DEBUG)		
													mul_task_debug();
												#endif
							Erase_The_Flash(&(COMMANDBUF[ComBufNum]->comcontent[7]));
			
							GoToNextStep();
				break;
/*************************************************************************************************
状态编号：25
****************************************************************************************************/
			case SetRTC:
											
			
								Time_Waited_Configured=Time_Regulate(&(COMMANDBUF[ComBufNum]->comcontent[7]));//将指令中的年月日时分秒信息赋值给特定全局变量，同时转换成RTC计数值返回
								if(Time_Waited_Configured !=0)//如果授时范围正确
								{
									Time_Adjust( Time_Waited_Configured);//修改RTC时钟计数器
									FPGA_SendWave_Init();//
								}
								
													#if	defined (USE_MC_DEBUG) && defined (PRINTF_DEBUG)		
														mul_task_debug();
													#endif
									GoToNextStep();
				break;
/*************************************************************************************************
状态编号：26
****************************************************************************************************/							
			case ThresHoldSet:
													#if	defined (USE_MC_DEBUG) && defined (PRINTF_DEBUG)		
														mul_task_debug();
													#endif
							
								write(0x15,COMMANDBUF[ComBufNum]->comcontent[6]);
								write(0x16,COMMANDBUF[ComBufNum]->comcontent[7]);
							
								/*********************临时调试FLASH***************************/
////////								LoadDataBufBasicInf(SendBufThree);
////////								df_read_open(0);
////////								df_read(&SendBufThree[8],1008);
////////			          Usart_send(SendBufThree,1018,COMMANDBUF[ComBufNum]->com_from,0,1);
////////			          
////////			          df_read_open(1008);
////////								df_read(&SendBufThree[8],1008);
////////			          Usart_send(SendBufThree,1018,COMMANDBUF[ComBufNum]->com_from,0,1);
////////			
////////			          df_read_open(2016);
////////								df_read(&SendBufThree[8],1008);
////////			          Usart_send(SendBufThree,1018,COMMANDBUF[ComBufNum]->com_from,0,1);
////////			
////////			          df_read_open(3024);
////////								df_read(&SendBufThree[8],1008);
////////			          Usart_send(SendBufThree,1018,COMMANDBUF[ComBufNum]->com_from,0,1);
////////								
////////								
////////								df_read_open(4032);
////////								df_read(&SendBufThree[8],63);
////////			          Usart_send(SendBufThree,73,COMMANDBUF[ComBufNum]->com_from,1,1);
							/***********************************************************/
							GoToNextStep();
				break;
/*************************************************************************************************
状态编号：27
****************************************************************************************************/	
			case LoadFlashDataAndTransUp:
							IWDG_Feed();//这步操作会耗费一定时间所以在这里喂一下看门狗
			
													#if	defined (USE_MC_DEBUG) && defined (PRINTF_DEBUG)		
														mul_task_debug();
													#endif
							LoadDataBufBasicInf(SendBufThree);
			        
              task_complete=New_LoadFlashData_And_TransUp(Flash_Addr2,Flash_Parabuf);
			        
							Check_CollectInf_TransupFlag();
							/*******************发送传输结束标志***************************/
			        FlashDataTransUp_Delay;//为了防止上位机粘包
			        SendBufThree[7]=0xff;
			        Usart_send(SendBufThree,10,COMMANDBUF[ComBufNum]->com_from,1,1);
			
							GoToNextStep();
				break;
/*************************************************************************************************
状态编号：28
****************************************************************************************************/
			case EraseTheBackupRegister:		
											#if	defined (USE_MC_DEBUG) && defined (PRINTF_DEBUG)		
													mul_task_debug();
												#endif
							Erase_The_BackupRegister(&(COMMANDBUF[ComBufNum]->comcontent[7]));
			
							GoToNextStep();
				break;
/*************************************************************************************************
状态编号：29
****************************************************************************************************/
case GetParamAboutLoadFlashInf:		
											IWDG_Feed();//这步操作会耗费一定时间所以在这里喂一下看门狗
											#if	defined (USE_MC_DEBUG) && defined (PRINTF_DEBUG)
											printf("\r\n GET ADDR TWO ");
											#endif
											Flash_Addr2=Get_AddrTwo(&(COMMANDBUF[ComBufNum]->comcontent[12]));
											if(Flash_Addr2 != 0xffffffff)
											{
												#if	defined (USE_MC_DEBUG) && defined (PRINTF_DEBUG)
												printf("\r\n GET ADDR ONE ");
												#endif
												task_complete=Get_AddrOne(&(COMMANDBUF[ComBufNum]->comcontent[6]),Flash_Addr2,Flash_Parabuf);
											}
											#if	defined (USE_MC_DEBUG) && defined (PRINTF_DEBUG)		
													mul_task_debug();
												#endif
							if(Flash_Addr2 == 0xffffffff )
								{
									LoadDataBufBasicInf(SendBufThree);
									/*******************发送传输结束标志***************************/
									SendBufThree[7]=0xff;
									Usart_send(SendBufThree,10,COMMANDBUF[ComBufNum]->com_from,1,1);
									tastate=THEEND;
								}
							else
								{GoToNextStep();}
				break;
/*************************************************************************************************
状态编号：30
****************************************************************************************************/
			case GetLaunchAnnouncement:		
											#if	defined (USE_MC_DEBUG) && defined (PRINTF_DEBUG)		
													mul_task_debug();
												#endif
			
							GoToNextStep();
				break;							
								
			default:
												#if	defined (USE_MC_DEBUG) && defined (PRINTF_DEBUG)		
													mul_task_debug();
												#endif
					tastate=THEEND;
				break;
		}

}

void GoToNextStep(void)
{		
					task_step=Get_Step(Com_Param,Aim_Param,step_num);
					if(task_step<=MAX_TaskNum) {tastate=(TakeAction_typedef)(task_step);}
					else 
						{
							#if	defined (USE_MC_DEBUG) && defined (PRINTF_DEBUG)		
									printf("\r\n MCU don't know the meaning of the task_step\r\n");
							#endif
							tastate=THEEND;
						}
					step_num=step_num+1;
}

uint8_t Basic_Inf_Set(uint8_t *combufnum)
{	
		uint8_t tempery_buf[2];
		TerminalBasicInf->LocalTerminalAddr				=combufnum[6];
		TerminalBasicInf->FrontFrontTerminalAddr 	=combufnum[7];
		TerminalBasicInf->FrontTerminalAddr				=combufnum[8];
		TerminalBasicInf->NextTerminalAddr				=combufnum[9];
		TerminalBasicInf->NextNextTerminalAddr 		=combufnum[10];
	
//		/**************************将新的基本信息配置存入FLASH***********************************/
		BackupRegister_WriteBytes(BasicInf_recbuf,FlashBackup_TerminalBasicInf_Addr,6);

		tempery_buf[0]=0;
		tempery_buf[1]=1;
		BackupRegister_WriteBytes(tempery_buf,10,2);
		return 1;
			
		
		
}
void LoadDataBufBasicInf(uint8_t *databuf)//
{
	databuf[0]=0x66;
	databuf[1]=0xcc;
	/*********************装载原地址和目的地址******************************/
	if(COMMANDBUF[ComBufNum]->com_from ==1 )//该指令由PC发出
		{
				databuf[4]=TerminalBasicInf->LocalTerminalAddr;//oriaddr
				databuf[5]=0xff;//desaddr
		}
	else //该指令由相邻终端发出
		{
				databuf[4]=TerminalBasicInf->LocalTerminalAddr;//oriaddr改为本端地址
				if(COMMANDBUF[ComBufNum]->com_direction == 1)//正向
					{		
						databuf[5]=TerminalBasicInf->FrontTerminalAddr;
					}
				else if(COMMANDBUF[ComBufNum]->com_direction == 2)//反向
					{				
						databuf[5]=TerminalBasicInf->NextTerminalAddr;
					}
			
		}
	
	/*******************装载指令类型*********************/
		databuf[6]=COMMANDBUF[ComBufNum]->comtype;//type
		/*******************装载本端地址***********************/
		databuf[7]=TerminalBasicInf->LocalTerminalAddr;
}

uint8_t LoadBasicInf(uint16_t length)
{
//	uint8_t i;
//	uint16_t sum=0;
//	SendBufOne[2]=(uint8_t)(length>>8);
//	SendBufOne[3]=(uint8_t)(length);
	
	if(COMMANDBUF[ComBufNum]->com_from ==1 )//该指令由PC发出
		{
				SendBufOne[4]=TerminalBasicInf->LocalTerminalAddr;//oriaddr
				SendBufOne[5]=0xff;//desaddr
		}
	else //该指令由相邻终端发出
		{
				SendBufOne[4]=TerminalBasicInf->LocalTerminalAddr;//oriaddr改为本端地址
				if(COMMANDBUF[ComBufNum]->com_direction == 1)//正向
					{		
						SendBufOne[5]=TerminalBasicInf->FrontTerminalAddr;
					}
				else if(COMMANDBUF[ComBufNum]->com_direction == 2)//反向
					{				
						SendBufOne[5]=TerminalBasicInf->NextTerminalAddr;
					}
			
		}
	SendBufOne[6]=COMMANDBUF[ComBufNum]->comtype;//type
		
	SendBufOne[7]=TerminalBasicInf->LocalTerminalAddr;
	SendBufOne[8]=TerminalBasicInf->FrontFrontTerminalAddr;
	SendBufOne[9]=TerminalBasicInf->FrontTerminalAddr;
	SendBufOne[10]=TerminalBasicInf->NextTerminalAddr;
	SendBufOne[11]=TerminalBasicInf->NextNextTerminalAddr;
	SendBufOne[12]=TerminalBasicInf->FlashOkOrNot;
		
	return 1;
}

uint8_t LoadDGInf(uint16_t length)
{
//	uint8_t i;
//	uint16_t sum=0;
//	SendBufOne[2]=(uint8_t)(length>>8);
//	SendBufOne[3]=(uint8_t)(length);
	
	if(step_num < 6)
		{
			SendBufOne[6]=COMMANDBUF[ComBufNum]->comtype;//type
			if(COMMANDBUF[ComBufNum]->com_from ==1 )//该指令由PC发出
				{
						SendBufOne[4]=TerminalBasicInf->LocalTerminalAddr;//oriaddr
						SendBufOne[5]=0xff;//desaddr
				}
			else //该指令由相邻终端发出
				{
						SendBufOne[4]=TerminalBasicInf->LocalTerminalAddr;//oriaddr改为本端地址
						if(COMMANDBUF[ComBufNum]->com_direction == 1)//正向
							{		
								SendBufOne[5]=TerminalBasicInf->FrontTerminalAddr;
							}
						else if(COMMANDBUF[ComBufNum]->com_direction == 2)//反向
							{				
								SendBufOne[5]=TerminalBasicInf->NextTerminalAddr;
							}
					
				}
		
		}
	
	else //说明是备用的行动步骤中调用了这个函数，此时COMMANDBUF[ComBufNum]或许会失效
		{
				SendBufOne[6]=WAITDATABUF[waitData_Trigerednum]->command_type;//type
				if(WAITDATABUF[waitData_Trigerednum]->command_comfrom ==1 )//该指令由PC发出
				{
						SendBufOne[4]=TerminalBasicInf->LocalTerminalAddr;//oriaddr
						SendBufOne[5]=0xff;//desaddr
				}
			else //该指令由相邻终端发出
				{
						SendBufOne[4]=TerminalBasicInf->LocalTerminalAddr;//oriaddr改为本端地址
						if(WAITDATABUF[waitData_Trigerednum]->command_direction == 1)//正向
							{		
								SendBufOne[5]=TerminalBasicInf->FrontTerminalAddr;
							}
						else if(WAITDATABUF[waitData_Trigerednum]->command_direction == 2)//反向
							{				
								SendBufOne[5]=TerminalBasicInf->NextTerminalAddr;
							}
					
				}
		
		}
		
		
		
		
	
	SendBufOne[7]=TerminalBasicInf->LocalTerminalAddr;
	SendBufOne[8]=DG_StateOne;
	SendBufOne[9]=DG_StateTwo;
		
		
	return 1;

}

void LoadDGState_inf(void)
{
	DGStateInfBuf[0]=DG_StateOne;
	DGStateInfBuf[1]=DG_StateTwo;
}

void Usart_send(uint8_t * sendbuf,uint16_t length,uint8_t unique_ID,uint8_t checksum,uint8_t com_dat)
{
	uint16_t i,j,sum=0;
	
	
	if(checksum==1)//需要计算校验和
		{
			if( com_dat == 1) //data装载两位校验和以及2位数据长度
			{
				sendbuf[2]=(uint8_t)(length>>8);
				sendbuf[3]=(uint8_t)(length);
				
				WLuart_header_addr=sendbuf[5];
				
				for(i=0;i<length-2;i++)
					{
						sum=sum+sendbuf[i];
					}
				sendbuf[length-2]=(uint8_t)(sum>>8);
				sendbuf[length-1]=(uint8_t)(sum);
			}
			else //command装载一位校验和以及一位指令长度
			{
				sendbuf[2]=(uint8_t)(length);
				
				for(i=0;i<length-1;i++)
					{
						sum=sum+sendbuf[i];
					}	
				sendbuf[length-1]=(uint8_t)(sum);
			}
	
		}
		
	if(unique_ID == 1) //回应给上位机
		{
			for (i=0;i<length;i++)
				{
					while(USART_GetFlagStatus(USART2, USART_FLAG_TC)==RESET);
					USART_SendData(USART2, sendbuf[i]);				
				}
		}
	else //回应给普通终端
		{
//			Send_WLUare_Header(WLuart_header_addr,0x02);
//			for (i=0;i<length;i++)
//				{
//					while(USART_GetFlagStatus(USART3, USART_FLAG_TC)==RESET);
//					USART_SendData(USART3, sendbuf[i]);				
//				}
//			printf("\r\n  send data by usart3\r\n");
			for(i=0;i<length;)
					{
						#ifdef USE_WLUART
						while(GPIO_ReadInputDataBit(AUX_PORT,AUX_PIN) == 0); 
						#endif
						Send_WLUare_Header(WLuart_header_addr,WLuart_Channel);
						for(j=0;j<SubPackage_Size;j++)
							{
								if(i<length)
									{
									while(USART_GetFlagStatus(USART3, USART_FLAG_TC)==RESET);
									USART_SendData(USART3, sendbuf[i]);	
									}
								else
									{j=SubPackage_Size;}
								i++;
							}
							
					}
		
		}
}

void ChangeCombuf_BeforeTransDown(uint8_t *content,uint8_t direc)
{
//	if(direc==1) //从小号终端到大号终端
//		{
//			content[3]
//		}
//	else if(direc==2) //从大号终端到小号终端
//		{
//		
//		}

}

void DG_Check(void)
{
	uint8_t a,b,num;
	uint16_t i;
	if(DG_checkcounter==2000000){DG_checkcounter=0;}
	else {DG_checkcounter++;}
			
/**************************************************************************************/
	a = read(0x14) & 0x8C;	//读发送标志
			if(a != 0)
			{
				if(a == 0x80)
				{
					write(0x0D,0x80);
					write(0x0D,0x00);	//清prepare_tx_flag标志
				Launch_Announcement();//超声信号发射通告，发两次，一个向前端发，一个向后端发
				}
				else if(a == 0x08)
				{
					tx_flag1 = 1;
					LED3_ON;
				}
				else if(0x04)
				{
					tx_flag2 = 1;
					LED3_ON;
				}
				
			}
			else
			{
				if(tx_flag1 == 1)
				{
					tx_flag1 = 0;
					LocalSignalAmplitudeInfBuf[0]=read(0x21);
					LocalSignalAmplitudeInfBuf[1]=read(0x22);
//////////////											#if	defined (USE_MC_DEBUG) && defined (PRINTF_DEBUG)		
//////////////													printf(" LocalSignalAmplitudeInfBuf[0]= %2x\r\n",LocalSignalAmplitudeInfBuf[0]);
//////////////													printf(" LocalSignalAmplitudeInfBuf[1]= %2x\r\n",LocalSignalAmplitudeInfBuf[1]);
//////////////											#endif
//					USART_SendData(USART2, read(0x21));while(USART_GetFlagStatus(USART2, USART_FLAG_TC)==RESET);
//					USART_SendData(USART2, read(0x22));while(USART_GetFlagStatus(USART2, USART_FLAG_TC)==RESET);
				}
				else if(tx_flag2 == 1)
				{
					tx_flag2 = 0;
					LocalSignalAmplitudeInfBuf[2]=read(0x23);
					LocalSignalAmplitudeInfBuf[3]=read(0x24);
					
////////////											#if	defined (USE_MC_DEBUG) && defined (PRINTF_DEBUG)		
////////////													printf(" LocalSignalAmplitudeInfBuf[2]= %2x\r\n",LocalSignalAmplitudeInfBuf[2]);
////////////													printf(" LocalSignalAmplitudeInfBuf[3]= %2x\r\n",LocalSignalAmplitudeInfBuf[3]);
////////////											#endif
//					USART_SendData(USART2, read(0x23));while(USART_GetFlagStatus(USART2, USART_FLAG_TC)==RESET);
//					USART_SendData(USART2, read(0x24));while(USART_GetFlagStatus(USART2, USART_FLAG_TC)==RESET);
				}
				LED3_OFF;
			}
/**************************************************************************************/			
					if(Led2_Blink_Times == 0)
					{
						a = read(0x15) & 0x88;//8;	//收到的其他终端输出方波闪灯
						if(a == 0x80 || a == 0x08)
							{
								LED2_OFF;
							}
						else
							{
								LED2_ON;
							}
						a = 0;
					}
				
/**************************************************************************************/		
			a = read(0x0D) & 0x80;	//正常接收1
			if(a == 0x80)
			{
				a=read(0x25);
											#if	defined (USE_MC_DEBUG) && defined (PRINTF_DEBUG)		
													printf(" receive one ,a = %2x\r\n",a);
											#endif
				if((a&0xf0)== 0)//高四位为0
					{DG_StateOne=DG_StateOne & 0x0f;}//高4位赋值0
				if((a&0x0f)== 0)//低四位为0
					{DG_StateOne=DG_StateOne & 0xf0;}//低4位赋值0
					
					
				if((a&0xf0)== 8)//高四位为0
					{
					 WaveFromFarAway=1;
											#if	defined (USE_MC_DEBUG) && defined (PRINTF_DEBUG)		
													printf(" receive wave from far away ,up,right\r\n");
											#endif
					}//
				if((a&0x0f)== 8)//低四位为0
					{
						WaveFromFarAway=1;
											#if	defined (USE_MC_DEBUG) && defined (PRINTF_DEBUG)		
													printf(" receive wave from far away ,up,left\r\n");
											#endif
					}//
				
				write(0x0E,0x09);		//目的：触发超时计数，设计在收到有效数据后重新开始超时计数
				write(0x0E,0x00);
				
//				USART_SendData(USART2, 0x00);while(USART_GetFlagStatus(USART2, USART_FLAG_TC)==RESET);
//				USART_SendData(USART2, 0x00);while(USART_GetFlagStatus(USART2, USART_FLAG_TC)==RESET);
//				USART_SendData(USART2, 0x08);while(USART_GetFlagStatus(USART2, USART_FLAG_TC)==RESET);
//				USART_SendData(USART2, 0x00);while(USART_GetFlagStatus(USART2, USART_FLAG_TC)==RESET);
				
				Led1_Blink_Times=4;
				Led1_counter=0;
				Led1_Freq_Param=2;
				a = 0;
			}
			
			a = read(0x0E) & 0x80;	//正常接收2
			if(a == 0x80)
			{
				a=read(0x26);
											#if	defined (USE_MC_DEBUG) && defined (PRINTF_DEBUG)		
													printf(" receive two ,a = %2x\r\n",a);
											#endif
				if((a&0xf0)== 0)//高四位为0
					{DG_StateTwo=DG_StateTwo & 0x0f;}//高4位赋值0
				if((a&0x0f)== 0)//低四位为0
					{DG_StateTwo=DG_StateTwo & 0xf0;}//低4位赋值0
					
				if((a&0xf0)== 8)//高四位为0
					{
						WaveFromFarAway=1;
											#if	defined (USE_MC_DEBUG) && defined (PRINTF_DEBUG)		
													printf(" receive wave from far away ,down,right\r\n");
											#endif
					}//
				if((a&0x0f)== 8)//低四位为0
					{
						WaveFromFarAway=1;
											#if	defined (USE_MC_DEBUG) && defined (PRINTF_DEBUG)		
													printf(" receive wave from far away ,down,left\r\n");
											#endif
					}//
				
				write(0x10,0x09);		//目的：触发超时计数，设计在收到有效数据后重新开始超时计数
				write(0x10,0x00);
			
				Led1_Blink_Times=4;
				Led1_counter=0;
				Led1_Freq_Param=2;
			
				a = 0;
			}
			
/**************************************************************************************/
			a = read(0x1E);//查询幅值信息是否可读
			if(a != 0)
			{	
				if(a == 0x10 ) //取上轨右侧幅值
					{
						position_mul=1;
						read_dat(3,4,6);
					}
				else if (a == 0x80 ) //取上轨左侧幅值
					{
						position_mul=0;
						read_dat(3,4,6);
					}
				else if (a == 0x01 ) //取下轨右侧幅值
					{
						position_mul=1;
						read_dat(4,4,6);
					}
				else if (a == 0x08 ) //取下轨左侧幅值
					{
						position_mul=0;
						read_dat(4,4,6);
					}
				
				write(0x14,0x88);//清可读标志
				write(0x14,0x00);//
				a = 0;
			}
			
			
/*****************************超时判断***************************************/
			a = read(0x27) & 0x99;	//
			if(a != 0)
			{
				b = read(0x05);
				num = b << 8;
				
				b = read(0x06);
				num = num + b;
				Rec_num[timeout_cnt] = num;
				//USART_SendData(USART2, Rec_num[t]>>8);while(USART_GetFlagStatus(USART2, USART_FLAG_TC)==RESET);
				//USART_SendData(USART2, Rec_num[t]);while(USART_GetFlagStatus(USART2, USART_FLAG_TC)==RESET);
				if(num > 0)	timeout_cnt++;
				
				if(a == 0x80)					//左上超时
				{				
											#if	defined (USE_MC_DEBUG) && defined (PRINTF_DEBUG)		
													printf(" left up time out \r\n");
											#endif
					DG_StateOne=(DG_StateOne & 0xf0) | 0x07;//低4位赋值7
					write(0x11,0x80);
					
					/****************************本次超时没有幅值，要把上一次得到的幅值信息清零****************************/
					for(i=0;i<16;i++)
						{
							SignalAmplitudeInfBuf[i]=0;
						}
					/**************************************************************************/
					Led4_Blink_Times=2;
					Led4_counter=0;
					Led4_Freq_Param=8;
				}
				else if(a == 0x08)		//左下超时
				{
											#if	defined (USE_MC_DEBUG) && defined (PRINTF_DEBUG)		
													printf(" left down time out \r\n");
											#endif
					DG_StateTwo=(DG_StateTwo & 0xf0) | 0x07;//低4位赋值7
					write(0x11,0x80);
					
						/****************************本次超时没有幅值，要把上一次得到的幅值信息清零****************************/
					for(i=0;i<16;i++)
						{
							SignalAmplitudeInfBuf[i+32]=0;
						}
					/**************************************************************************/
					Led4_Blink_Times=2;
					Led4_counter=0;
					Led4_Freq_Param=8;
				}
				else if(a == 0x10)		//右上超时
				{
											#if	defined (USE_MC_DEBUG) && defined (PRINTF_DEBUG)		
													printf(" right up time out \r\n");
											#endif
					DG_StateOne=(DG_StateOne & 0x0f) | 0x70;//高4位赋值7
					write(0x11,0x80);
					
						/****************************本次超时没有幅值，要把上一次得到的幅值信息清零****************************/
					for(i=0;i<16;i++)
						{
							SignalAmplitudeInfBuf[i+16]=0;
						}
					/**************************************************************************/
					Led4_Blink_Times=2;
					Led4_counter=0;
					Led4_Freq_Param=8;
				}
				else if(a == 0x01)		//右下超时
				{
											#if	defined (USE_MC_DEBUG) && defined (PRINTF_DEBUG)		
													printf(" right down time out \r\n");
											#endif
					DG_StateTwo=(DG_StateTwo & 0x0f) | 0x70;//高4位赋值7
					write(0x11,0x80);
					
						/****************************本次超时没有幅值，要把上一次得到的幅值信息清零****************************/
					for(i=0;i<16;i++)
						{
							SignalAmplitudeInfBuf[i+48]=0;
						}
					/**************************************************************************/
					Led4_Blink_Times=2;
					Led4_counter=0;
					Led4_Freq_Param=8;
				}
				
				
				else if(a == 0x11)		//右下右上同时超时
				{
											#if	defined (USE_MC_DEBUG) && defined (PRINTF_DEBUG)		
													printf(" right up and down time out \r\n");
											#endif
					DG_StateOne=(DG_StateOne & 0x0f) | 0x70;//高4位赋值7
					DG_StateTwo=(DG_StateTwo & 0x0f) | 0x70;//高4位赋值7
					write(0x11,0x80);
					
						/****************************本次超时没有幅值，要把上一次得到的幅值信息清零****************************/
					for(i=0;i<16;i++)
						{
							SignalAmplitudeInfBuf[i+16]=0;
						}
					for(i=0;i<16;i++)
						{
							SignalAmplitudeInfBuf[i+48]=0;
						}
					/**************************************************************************/
					Led4_Blink_Times=2;
					Led4_counter=0;
					Led4_Freq_Param=8;
				}
				
				else if(a == 0x88)		//左下左上同时超时
				{
											#if	defined (USE_MC_DEBUG) && defined (PRINTF_DEBUG)		
													printf(" left up and down time out \r\n");
											#endif
					DG_StateOne=(DG_StateOne & 0xf0) | 0x07;//低4位赋值7
					DG_StateTwo=(DG_StateTwo & 0xf0) | 0x07;//低4位赋值7
					write(0x11,0x80);
					
						/****************************本次超时没有幅值，要把上一次得到的幅值信息清零****************************/
					for(i=0;i<16;i++)
						{
							SignalAmplitudeInfBuf[i]=0;
						}
					for(i=0;i<16;i++)
						{
							SignalAmplitudeInfBuf[i+32]=0;
						}
					
					/**************************************************************************/
					Led4_Blink_Times=2;
					Led4_counter=0;
					Led4_Freq_Param=8;
				}
				
				
				else if(a == 0x99)		//上下左右全都超时
				{
											#if	defined (USE_MC_DEBUG) && defined (PRINTF_DEBUG)		
													printf(" all time out \r\n");
											#endif
					DG_StateOne=0x77;
					DG_StateTwo=0x77;//低4位赋值7
					write(0x11,0x80);
					
						/****************************本次超时没有幅值，要把上一次得到的幅值信息清零****************************/
					for(i=0;i<64;i++)
						{
							SignalAmplitudeInfBuf[i]=0;
						}
					
					/**************************************************************************/
					Led4_Blink_Times=2;
					Led4_counter=0;
					Led4_Freq_Param=8;
				}
	
				write(0x11,0x00);
				a = 0;
			}

}

void DataContentComnbined(uint8_t *sendbuf,uint16_t sendbuflength,uint16_t Comnbinedlength,uint8_t *Comnbinedbuf)
{
		uint16_t totalength,i;
	
		totalength=sendbuflength+Comnbinedlength;
//		Comnbinedlength_copy=Comnbinedlength;
	
		sendbuf[2]=(uint8_t)(totalength>>8);
		sendbuf[3]=(uint8_t)(totalength);
		
		for(i=0;i<Comnbinedlength;i++)
			{
					sendbuf[sendbuflength-2+i]=Comnbinedbuf[i];	
			}
}

uint8_t WaitIme_Ack(uint8_t getackflag,uint8_t direct,uint8_t Imeacktype,uint8_t comdirect,uint8_t comtype)
{
//	printf("\r\n  the getackflag is %d the direction is %d the command direction is %d\r\n",getackflag,direct,comdirect);
//	printf("\r\n  the Imeacktype is %2x the comtype is %2x\r\n",Imeacktype,comtype);
	
	if(getackflag == 1) {GetImmediaAck_Flag=0;}
	if(getackflag == 1 && Imeacktype == comtype)
		{
			
			if(direct==1 && comdirect ==2) {return 1;}
			else if (direct==2 && comdirect ==1) {return 1;}
			else {return 0;}
		}
	else
		{
			return 0;
		}
}

uint8_t WaitData_Ack(DataRestore_typedef *databuf,uint8_t comdirect,uint8_t comtype)
{
	if((databuf[0].datatype== comtype || databuf[0].datatype == 0x88 )&& databuf[0].data_used == 0 && databuf[0].data_direction !=comdirect)//等待的数据回应可能是正常的waitdattype，也可能是异常的0x88
		{databuf[0].data_read =1; return 0;}
	else if((databuf[1].datatype== comtype  || databuf[1].datatype == 0x88 )&& databuf[1].data_used == 0 && databuf[0].data_direction !=comdirect)
		{databuf[1].data_read =1; return 1;}
	else if((databuf[2].datatype== comtype  || databuf[2].datatype == 0x88 )&& databuf[2].data_used == 0 && databuf[0].data_direction !=comdirect)
		{databuf[2].data_read =1; return 2;}
	else
			{return 3;}

}

void ImeAck_Time_Param_Init(uint16_t waitime)//等待waitime*10毫秒
{
	GetImmediaAck_Flag=0;
	ImeAck_Direction=0;
	ImeAckOk_Flag=0;
	ImeAckType=0XFF;
	
	WaitDataTime=waitime;
	
	/*************这样做相当于清空接收缓冲区*********************/
	DATABUF[0]->data_read =1;
	DATABUF[0]->data_used	=1;
	DATABUF[1]->data_read =1;
	DATABUF[1]->data_used	=1;
	DATABUF[2]->data_read =1;
	DATABUF[2]->data_used	=1;
	/************************************************/

}


void LoadErrorReportInf(uint16_t length)
{
	SendBufOne[2]=(uint8_t)(length>>8);
	SendBufOne[3]=(uint8_t)(length);
	
	if(WAITDATABUF[waitData_Trigerednum]->command_comfrom ==1 )//该指令由PC发出
		{
				SendBufOne[4]=TerminalBasicInf->LocalTerminalAddr;//oriaddr
				SendBufOne[5]=0xff;//desaddr
				if(WAITDATABUF[waitData_Trigerednum]->command_direction == 1)//正向
					{		
						SendBufOne[7]=TerminalBasicInf->NextTerminalAddr;//出错的是后一端
					}
				else if(WAITDATABUF[waitData_Trigerednum]->command_direction == 2)//反向
					{				
						SendBufOne[7]=TerminalBasicInf->FrontTerminalAddr;//出错的是前一端
					}
		}
	else //该指令由相邻终端发出
		{
				SendBufOne[4]=TerminalBasicInf->LocalTerminalAddr;//oriaddr改为本端地址
				if(WAITDATABUF[waitData_Trigerednum]->command_direction == 1)//正向
					{		
						SendBufOne[5]=TerminalBasicInf->FrontTerminalAddr;
						SendBufOne[7]=TerminalBasicInf->NextTerminalAddr;//出错的是后一端
					}
				else if(WAITDATABUF[waitData_Trigerednum]->command_direction == 2)//反向
					{				
						SendBufOne[5]=TerminalBasicInf->NextTerminalAddr;
						SendBufOne[7]=TerminalBasicInf->FrontTerminalAddr;//出错的是前一端
					}
			
		}
	SendBufOne[6]=0x88;//ErrorReporType

}

void LoadRcc_Inf(void)
{
//	RccInfBuf[0]=(uint8_t)(Year>>8);
//	RccInfBuf[1]=(uint8_t)Year;
//	RccInfBuf[2]=Month;
//	RccInfBuf[3]=Day;
//	RccInfBuf[4]=Hour;
//	RccInfBuf[5]=Minute;
//	RccInfBuf[6]=Second;
	
	RccInfBuf[0]=(uint8_t)(Year-2000);
	RccInfBuf[1]=Month;
	RccInfBuf[2]=Day;
	RccInfBuf[3]=Hour;
	RccInfBuf[4]=Minute;
	RccInfBuf[5]=Second;
}

void LoadStress_Inf(uint8_t OverTime)
{
	if(OverTime==1)
		{
			StressInfBuf[0]=0xff;
			StressInfBuf[1]=0xff;
			StressInfBuf[2]=0xff;
			StressInfBuf[3]=0xff;
		}
	else 
		{
			StressInfBuf[0]=MAX485_Recbuf[7];
			StressInfBuf[1]=MAX485_Recbuf[8];
			StressInfBuf[2]=MAX485_Recbuf[9];
			StressInfBuf[3]=MAX485_Recbuf[10];
		}

}

void LoadTemperature_Inf(void)
{
	uint16_t board_temperature;
	uint8_t  simple_boardtemperature;
	
	TemperatureInfBuf[2]=WaveFromFarAway;
	WaveFromFarAway=0;
	
	board_temperature=ReadTempurature();	
	
	simple_boardtemperature=(uint8_t)((board_temperature&0x0F00)>>4)+(uint8_t)((board_temperature&0x00F0)>>4);
	
	if((board_temperature&0xf000)==0xf000) //温度是0下的
		{
			simple_boardtemperature=255-simple_boardtemperature;
			simple_boardtemperature=simple_boardtemperature|0x80;//1000_0000
			TemperatureInfBuf[0]=simple_boardtemperature;
//			TemperatureInfBuf[2]=simple_boardtemperature;
		}
	else //温度是0上的
		{
			TemperatureInfBuf[0]=simple_boardtemperature;
//			TemperatureInfBuf[2]=simple_boardtemperature;
		}
	
		board_temperature=ReadTempurature_OUT();	
	
		simple_boardtemperature=(uint8_t)((board_temperature&0x0F00)>>4)+(uint8_t)((board_temperature&0x00F0)>>4);
	
	if((board_temperature&0xf000)==0xf000) //温度是0下的
		{
			simple_boardtemperature=255-simple_boardtemperature;
			simple_boardtemperature=simple_boardtemperature|0x80;//1000_0000
			TemperatureInfBuf[1]=simple_boardtemperature;
			TemperatureInfBuf[3]=simple_boardtemperature;
		}
	else //温度是0上的
		{
			TemperatureInfBuf[1]=simple_boardtemperature;
			TemperatureInfBuf[3]=simple_boardtemperature;
		}
								#if	defined (USE_MC_DEBUG) && defined (PRINTF_DEBUG)		
									printf("\r\n  the board temperature is %d,the outside Temperature is %d\r\n"
									,(TemperatureInfBuf[0]&0x7f),(TemperatureInfBuf[1]&0x7f));
								#endif

}

void LoadSignalAmplitude_Inf(void)
{
	uint16_t i;
	for(i=0;i<64;i++)
		{
			SignalAmplitudeInfBuf[i]=i;
		}

}

uint8_t LoadAllTheTrack_Inf(uint8_t length)
{
	uint16_t i;
	LoadDataBufBasicInf(SendBufOne);
	/*******************装载时间***********************/
	for(i=0;i<6;i++)
		{
		SendBufOne[i+8]=RccInfBuf[i];
		}
	/*******************装载通断信息***********************/
		SendBufOne[14]=DGStateInfBuf[0];
		SendBufOne[53]=DGStateInfBuf[1];
	/*******************装载应力信息***********************/
		SendBufOne[15]=StressInfBuf[0];
		SendBufOne[16]=StressInfBuf[1];
		SendBufOne[54]=StressInfBuf[2];
		SendBufOne[55]=StressInfBuf[3];
		/*******************装载温度信息***********************/
		SendBufOne[17]=TemperatureInfBuf[0];
		SendBufOne[18]=TemperatureInfBuf[1];
		SendBufOne[56]=TemperatureInfBuf[2];
		SendBufOne[57]=TemperatureInfBuf[3];
		/*******************装载本端信号幅值***********************/
		SendBufOne[19]=LocalSignalAmplitudeInfBuf[0];
		SendBufOne[20]=LocalSignalAmplitudeInfBuf[1];
		SendBufOne[58]=LocalSignalAmplitudeInfBuf[2];
		SendBufOne[59]=LocalSignalAmplitudeInfBuf[3];
		/*******************装载远端信号幅值信息***********************/
		for (i=0;i<32;i++)
			{
				SendBufOne[21+i]=SignalAmplitudeInfBuf[i];
				SendBufOne[60+i]=SignalAmplitudeInfBuf[i+32];
			
			}
	return 1;
}

void Launch_Announcement(void)
{
	SendBufTwo[3]=TerminalBasicInf->LocalTerminalAddr;
	SendBufTwo[5]=Launch_Announcement_Type;//type
	
	
	
	SendBufTwo[4]=TerminalBasicInf->FrontTerminalAddr;
	WLuart_header_addr=SendBufTwo[4];
	Usart_send(SendBufTwo,7,0,1,0);
	Delay(100);
	SendBufTwo[4]=TerminalBasicInf->NextTerminalAddr;
	WLuart_header_addr=SendBufTwo[4];
	Usart_send(SendBufTwo,7,0,1,0);//
	
	Launch_Announcement_Flag=210;

}

void Erase_The_Flash(uint8_t *combuf)
{
	uint16_t start_erase_addr,final_erase_addr;
	start_erase_addr=combuf[1]+(uint16_t)(combuf[0]<<8);
	final_erase_addr=combuf[3]+(uint16_t)(combuf[2]<<8);
	
												#if	defined (USE_MC_DEBUG) && defined (PRINTF_DEBUG)		
												printf("\r\n  start_erase_addr is %d ,final_erase_addr is %d \r\n"
												,start_erase_addr
												,final_erase_addr);
												#endif
	
	Flash_Erase(start_erase_addr,final_erase_addr);
}
void Erase_The_BackupRegister(uint8_t *combuf)
{
	uint8_t start_erase_addr,final_erase_addr,length;
	start_erase_addr=combuf[0];
	final_erase_addr=combuf[1];
	length=final_erase_addr-start_erase_addr+2;
	
	#if	defined (USE_MC_DEBUG) && defined (PRINTF_DEBUG)		
//												printf("\r\n  start_erase_addr is %d ,final_erase_addr is %d ,length is %d\r\n"
//												,start_erase_addr
//												,final_erase_addr
//												,length);
												#endif
	BackupRegister_WriteBytes(BackupRegEraseSentbuf,start_erase_addr,length);

}
void WaitData_Regist(uint8_t cd,uint8_t ct,uint8_t snc,uint8_t wdf,uint16_t wdtl,uint8_t com_comfrom,uint8_t ndr_flag,uint8_t wdr_quantity)
{
	if(WAITDATABUF[0]->WaitDataFlag == 0)
		{
			waitdata_registnum=1;
			
			WAITDATABUF[0]->command_direction=cd;
			WAITDATABUF[0]->command_type=ct;
			WAITDATABUF[0]->step_num_copy=snc;
			WAITDATABUF[0]->WaitDataFlag=wdf;
			WAITDATABUF[0]->noneed_dataresponse=ndr_flag;
//			waitdata_time_one=0;
			waitdata_time[0]=0;
			WAITDATABUF[0]->waitdata_timelimit=wdtl;
			WAITDATABUF[0]->comparam_copy=Com_Param;
			WAITDATABUF[0]->aimparam_copy=Aim_Param;
			WAITDATABUF[0]->command_comfrom=com_comfrom;
			WAITDATABUF[0]->WaitDataResponse_Quantity=wdr_quantity;
			
		}
	else if(WAITDATABUF[1]->WaitDataFlag == 0)
		{
			waitdata_registnum=2;
			
			WAITDATABUF[1]->command_direction=cd;
			WAITDATABUF[1]->command_type=ct;
			WAITDATABUF[1]->step_num_copy=snc;
			WAITDATABUF[1]->WaitDataFlag=wdf;
			WAITDATABUF[1]->noneed_dataresponse=ndr_flag;
			waitdata_time[1]=0;
			WAITDATABUF[1]->waitdata_timelimit=wdtl;
			WAITDATABUF[1]->comparam_copy=Com_Param;
			WAITDATABUF[1]->aimparam_copy=Aim_Param;
			WAITDATABUF[1]->command_comfrom=com_comfrom;
			WAITDATABUF[1]->WaitDataResponse_Quantity=wdr_quantity;
		
		}
	else if(WAITDATABUF[2]->WaitDataFlag == 0)
		{
			waitdata_registnum=3;
			
			WAITDATABUF[2]->command_direction=cd;
			WAITDATABUF[2]->command_type=ct;
			WAITDATABUF[2]->step_num_copy=snc;
			WAITDATABUF[2]->WaitDataFlag=wdf;
			WAITDATABUF[2]->noneed_dataresponse=ndr_flag;
			waitdata_time[2]=0;
			WAITDATABUF[2]->waitdata_timelimit=wdtl;
			WAITDATABUF[2]->comparam_copy=Com_Param;
			WAITDATABUF[2]->aimparam_copy=Aim_Param;
			WAITDATABUF[2]->command_comfrom=com_comfrom;
			WAITDATABUF[2]->WaitDataResponse_Quantity=wdr_quantity;
		
		}
	else
		{
			if(waitdata_registnum==3)//3号结构体刚刚被用过了，新的数据注册到3号结构体之后的一号结构体里
				{
					waitdata_registnum=1;
				
					WAITDATABUF[0]->command_direction=cd;
					WAITDATABUF[0]->command_type=ct;
					WAITDATABUF[0]->step_num_copy=snc;
					WAITDATABUF[0]->WaitDataFlag=wdf;
					WAITDATABUF[0]->noneed_dataresponse=ndr_flag;
					waitdata_time[0]=0;
					WAITDATABUF[0]->waitdata_timelimit=wdtl;
					WAITDATABUF[0]->comparam_copy=Com_Param;
					WAITDATABUF[0]->aimparam_copy=Aim_Param;
					WAITDATABUF[0]->command_comfrom=com_comfrom;
					WAITDATABUF[0]->WaitDataResponse_Quantity=wdr_quantity;
				
				}
			else if(waitdata_registnum==1)//1号结构体刚刚被用过了，新的数据注册到1号结构体之后的二号结构体里
				{
					waitdata_registnum=2;
				
				WAITDATABUF[1]->command_direction=cd;
				WAITDATABUF[1]->command_type=ct;
				WAITDATABUF[1]->step_num_copy=snc;
				WAITDATABUF[1]->WaitDataFlag=wdf;
				WAITDATABUF[1]->noneed_dataresponse=ndr_flag;
				waitdata_time[1]=0;
				WAITDATABUF[1]->waitdata_timelimit=wdtl;
				WAITDATABUF[1]->comparam_copy=Com_Param;
				WAITDATABUF[1]->aimparam_copy=Aim_Param;
				WAITDATABUF[1]->command_comfrom=com_comfrom;
				WAITDATABUF[1]->WaitDataResponse_Quantity=wdr_quantity;
				
				}
				else if(waitdata_registnum==2)//2号结构体刚刚被用过了，新的数据注册到2号结构体之后的三号结构体里
					{
						waitdata_registnum=3;
					
						WAITDATABUF[2]->command_direction=cd;
						WAITDATABUF[2]->command_type=ct;
						WAITDATABUF[2]->step_num_copy=snc;
						WAITDATABUF[2]->WaitDataFlag=wdf;
						WAITDATABUF[2]->noneed_dataresponse=ndr_flag;
						waitdata_time[2]=0;
						WAITDATABUF[2]->waitdata_timelimit=wdtl;
						WAITDATABUF[2]->comparam_copy=Com_Param;
						WAITDATABUF[2]->aimparam_copy=Aim_Param;
						WAITDATABUF[2]->command_comfrom=com_comfrom;
						WAITDATABUF[2]->WaitDataResponse_Quantity=wdr_quantity;
					
					}
		
		}
					#if	defined (USE_MC_DEBUG) && defined (PRINTF_DEBUG)		
						if(waitdata_registnum==1)
						{
							printf("\r\n  WAITDATABUF one ,its type is %2x ,flag is %d, step_numcopy is %d,compara is %d,aimpara is %d ,quantity is %d,no need data response flag is %d\r\n"
										,WAITDATABUF[0]->command_type
										,WAITDATABUF[0]->WaitDataFlag
										,WAITDATABUF[0]->step_num_copy
										,WAITDATABUF[0]->comparam_copy
										,WAITDATABUF[0]->aimparam_copy
										,WAITDATABUF[0]->WaitDataResponse_Quantity
										,WAITDATABUF[0]->noneed_dataresponse);	
						}
						else if(waitdata_registnum==2)
						{
							printf("\r\n  WAITDATABUF two ,its type is %2x ,flag is %d, step_numcopy is %d,compara is %d,aimpara is %d ,quantity is %d,no need data response flag is %d\r\n"
										,WAITDATABUF[1]->command_type
										,WAITDATABUF[1]->WaitDataFlag
										,WAITDATABUF[1]->step_num_copy
										,WAITDATABUF[1]->comparam_copy
										,WAITDATABUF[1]->aimparam_copy
										,WAITDATABUF[1]->WaitDataResponse_Quantity
										,WAITDATABUF[0]->noneed_dataresponse);	
						}
						else if(waitdata_registnum==3)
						{
							printf("\r\n  WAITDATABUF three ,its type is %2x ,flag is %d, step_numcopy is %d,compara is %d,aimpara is %d ,quantity is %d,no need data response flag is %d\r\n"
										,WAITDATABUF[2]->command_type
										,WAITDATABUF[2]->WaitDataFlag
										,WAITDATABUF[2]->step_num_copy
										,WAITDATABUF[2]->comparam_copy
										,WAITDATABUF[2]->aimparam_copy
										,WAITDATABUF[2]->WaitDataResponse_Quantity
										,WAITDATABUF[0]->noneed_dataresponse);	
						}
		
					
					#endif

}

void WaitData_Analysis(void)
{
	uint8_t buftwo_disable=0,bufthree_disable=0;
/********************************************************************/
				if(WAITDATABUF[0]->WaitDataFlag > 0 )
				{
					waitData_Trigerednum=0;
						if(WAITDATABUF[0]->WaitDataFlag == 2)//需要等待立即响应
							{
									ImeAckOk_Flag=WaitIme_Ack(GetImmediaAck_Flag,ImeAck_Direction,ImeAckType,
																							WAITDATABUF[0]->command_direction,
																							WAITDATABUF[0]->command_type);
								/************************还处在接收时限内********************************/
									if(WAITDATABUF[0]->waitdata_timelimit > waitdata_time[0] ) 
									{
										/*********************如果等到了立即响应就把WaitDataFlag置1，继续等待数据*********************************/
										if(ImeAckOk_Flag == 1)  
											{
													WAITDATABUF[0]->WaitDataFlag=1;
													WAITDATABUF[0]->waitdata_timelimit=300;//300代表等待数据的时间可以是很长，大概15秒
											}
											/***********等待时限内没收到想要的数据，那就继续去做其他事情*******************************************/
										else
											{tastate=TsakSW_Analy;}
									}
									/************************已经不在接收时限内了,清除等待数据的标志，同时跳转备用行动步骤********************************/
									else
									{
										WAITDATABUF[0]->WaitDataFlag=0;
										Com_Param=WAITDATABUF[0]->comparam_copy;
										Aim_Param=WAITDATABUF[0]->aimparam_copy;
										step_num=WAITDATABUF[0]->step_num_copy+4;
										
										/*****************已经产生的行动步骤不能被2，3注册区产生的行动步骤破坏掉*******************/
										buftwo_disable=1;
										bufthree_disable=1;
										/************************************************/
										GoToNextStep();
										
													#if	defined (USE_MC_DEBUG) && defined (PRINTF_DEBUG)		
																	printf("\r\n no ime ack, WAITDATABUF[0] cancel ,the waitdata type is %2x\r\n",WAITDATABUF[0]->command_type);
													#endif
									}
							}
						else if(WAITDATABUF[0]->WaitDataFlag == 1 && WAITDATABUF[0]->noneed_dataresponse == 1) //不需要等待数据
							{
							/*****************清空等待数据的标志，然后继续去做其他事情***********************/
								WAITDATABUF[0]->WaitDataFlag=0;
								tastate=TsakSW_Analy;
													#if	defined (USE_MC_DEBUG) && defined (PRINTF_DEBUG)		
																	printf("\r\n  WAITDATABUF[0] finished ,the waitdata type is %2x\r\n",WAITDATABUF[0]->command_type);
													#endif
							}
						else if(WAITDATABUF[0]->WaitDataFlag == 1 && WAITDATABUF[0]->noneed_dataresponse == 2) //可能需要等待失联报告
							{
									DataAckOk_Flag=WaitData_Ack(DATABUF[0],WAITDATABUF[0]->command_direction,0x88);
										/************************还处在接收时限内********************************/
									if(WAITDATABUF[0]->waitdata_timelimit > waitdata_time[0]) 
										{
											/*********************如果等到了想要的数据就用数据触发新的行动步骤，同时清除等待标志*********************************/
											if(DataAckOk_Flag < 3 )  
												{
														WAITDATABUF[0]->WaitDataFlag=0;
														
														DataPeriod=1;
														DatabufNum=DataAckOk_Flag;
													
														tastate=TA_OOM_Init;
														#if	defined (USE_MC_DEBUG) && defined (PRINTF_DEBUG)		
																	printf("\r\n  WAITDATABUF[0] finished ,the waitdata type is %2x\r\n",0x88);
													#endif
												}
												/***********等待时限内没收到想要的数据，那就继续去做其他事情*******************************************/
											else
												{tastate=TsakSW_Analy;}
										}
									/************************已经不在接收时限内了,清除等待数据的标志，然后继续做其他事情********************************/
									else
										{
											WAITDATABUF[0]->WaitDataFlag=0;
											tastate=TsakSW_Analy;
											
											#if	defined (USE_MC_DEBUG) && defined (PRINTF_DEBUG)		
																	printf("\r\n  WAITDATABUF[0] cancel ,the waitdata type is %2x\r\n",0x88);
													#endif
										}
							
							}
						else if(WAITDATABUF[0]->WaitDataFlag == 1  )//需要等待数据
							{
									DataAckOk_Flag=WaitData_Ack(DATABUF[0],WAITDATABUF[0]->command_direction,WAITDATABUF[0]->command_type);
										/************************还处在接收时限内********************************/
									if(WAITDATABUF[0]->waitdata_timelimit > waitdata_time[0]) 
										{
											/*********************如果等到了想要的数据就用数据触发新的行动步骤，同时清除等待标志*********************************/
											if(DataAckOk_Flag < 3 )  
												{
														WAITDATABUF[0]->WaitDataFlag=0;
														
														DataPeriod=1;
														DatabufNum=DataAckOk_Flag;
													
														/*****************已经产生的行动步骤不能被2，3注册区产生的行动步骤破坏掉*******************/
														buftwo_disable=1;
														bufthree_disable=1;
														/************************************************/
							
														tastate=TA_OOM_Init;
													#if	defined (USE_MC_DEBUG) && defined (PRINTF_DEBUG)		
																	printf("\r\n  WAITDATABUF[0] finished ,the waitdata type is %2x\r\n",WAITDATABUF[0]->command_type);
													#endif
												}
												/***********等待时限内没收到想要的数据，那就继续去做其他事情*******************************************/
											else
												{tastate=TsakSW_Analy;}
										}
									/************************已经不在接收时限内了,清除等待数据的标志，同时跳转备用行动步骤********************************/
									else
										{
											WAITDATABUF[0]->WaitDataFlag=0;
											Com_Param=WAITDATABUF[0]->comparam_copy;
											Aim_Param=WAITDATABUF[0]->aimparam_copy;
											step_num=WAITDATABUF[0]->step_num_copy+4;
											
											/*****************已经产生的行动步骤不能被2，3注册区产生的行动步骤破坏掉*******************/
														buftwo_disable=1;
														bufthree_disable=1;
														/************************************************/
											GoToNextStep();
											#if	defined (USE_MC_DEBUG) && defined (PRINTF_DEBUG)		
																	printf("\r\n  WAITDATABUF[0] cancel ,the waitdata type is %2x\r\n",WAITDATABUF[0]->command_type);
													#endif
										}
							
							}
				}

/********************************************************************/
			else if(WAITDATABUF[1]->WaitDataFlag > 0 && buftwo_disable == 0 )
				{
					waitData_Trigerednum=1;
						if(WAITDATABUF[1]->WaitDataFlag == 2)//需要等待立即响应
							{
									ImeAckOk_Flag=WaitIme_Ack(GetImmediaAck_Flag,ImeAck_Direction,ImeAckType,
																							WAITDATABUF[1]->command_direction,
																							WAITDATABUF[1]->command_type);
								/************************还处在接收时限内********************************/
									if(WAITDATABUF[1]->waitdata_timelimit > waitdata_time[1] ) 
									{
										/*********************如果等到了立即响应就把WaitDataFlag置1，继续等待数据*********************************/
										if(ImeAckOk_Flag == 1)  
											{
													WAITDATABUF[1]->WaitDataFlag=1;
													WAITDATABUF[1]->waitdata_timelimit=300;//300代表等待数据的时间可以是很长，大概15秒
											}
											/***********等待时限内没收到想要的数据，那就继续去做其他事情*******************************************/
										else
											{tastate=TsakSW_Analy;}
									}
									/************************已经不在接收时限内了,清除等待数据的标志，同时跳转备用行动步骤********************************/
									else
									{
										WAITDATABUF[1]->WaitDataFlag=0;
										Com_Param=WAITDATABUF[1]->comparam_copy;
										Aim_Param=WAITDATABUF[1]->aimparam_copy;
										step_num=WAITDATABUF[1]->step_num_copy+4;
										
														/*****************已经产生的行动步骤不能被3注册区产生的行动步骤破坏掉*******************/
														bufthree_disable=1;
														/************************************************/
										GoToNextStep();
										#if	defined (USE_MC_DEBUG) && defined (PRINTF_DEBUG)		
																	printf("\r\n no ime ack , WAITDATABUF[1] cancel ,the waitdata type is %2x\r\n",WAITDATABUF[1]->command_type);
													#endif
									}
							}
						else if(WAITDATABUF[1]->WaitDataFlag == 1 && WAITDATABUF[1]->noneed_dataresponse == 1) //不需要等待数据
							{
							/*****************清空等待数据的标志，然后继续去做其他事情***********************/
								WAITDATABUF[1]->WaitDataFlag=0;
								tastate=TsakSW_Analy;
								#if	defined (USE_MC_DEBUG) && defined (PRINTF_DEBUG)		
																	printf("\r\n  WAITDATABUF[1] finished ,the waitdata type is %2x\r\n",WAITDATABUF[1]->command_type);
													#endif
							}
						else if(WAITDATABUF[1]->WaitDataFlag == 1 && WAITDATABUF[1]->noneed_dataresponse == 2) //可能需要等待失联报告
							{
									DataAckOk_Flag=WaitData_Ack(DATABUF[0],WAITDATABUF[1]->command_direction,0x88);
										/************************还处在接收时限内********************************/
									if(WAITDATABUF[1]->waitdata_timelimit > waitdata_time[1]) 
										{
											/*********************如果等到了想要的数据就用数据触发新的行动步骤，同时清除等待标志*********************************/
											if(DataAckOk_Flag < 3 )  
												{
														WAITDATABUF[1]->WaitDataFlag=0;
														
														DataPeriod=1;
														DatabufNum=DataAckOk_Flag;
													
														tastate=TA_OOM_Init;
														#if	defined (USE_MC_DEBUG) && defined (PRINTF_DEBUG)		
																	printf("\r\n  WAITDATABUF[1] finished ,the waitdata type is %2x\r\n",0x88);
													#endif
												}
												/***********等待时限内没收到想要的数据，那就继续去做其他事情*******************************************/
											else
												{tastate=TsakSW_Analy;}
										}
									/************************已经不在接收时限内了,清除等待数据的标志，然后继续做其他事情********************************/
									else
										{
											WAITDATABUF[1]->WaitDataFlag=0;
											tastate=TsakSW_Analy;
											
											#if	defined (USE_MC_DEBUG) && defined (PRINTF_DEBUG)		
																	printf("\r\n  WAITDATABUF[1] cancel ,the waitdata type is %2x\r\n",0x88);
													#endif
										}
							
							}
						else if(WAITDATABUF[1]->WaitDataFlag == 1)//需要等待数据
							{
									DataAckOk_Flag=WaitData_Ack(DATABUF[0],WAITDATABUF[1]->command_direction,WAITDATABUF[1]->command_type);
										/************************还处在接收时限内********************************/
									if(WAITDATABUF[1]->waitdata_timelimit > waitdata_time[1] ) 
										{
											/*********************如果等到了想要的数据就用数据触发新的行动步骤，同时清除等待标志*********************************/
											if(DataAckOk_Flag < 3 )  
												{
														WAITDATABUF[1]->WaitDataFlag=0;
														
														DataPeriod=1;
														DatabufNum=DataAckOk_Flag;
													
													/*****************已经产生的行动步骤不能被3注册区产生的行动步骤破坏掉*******************/
														bufthree_disable=1;
														/************************************************/
													
														tastate=TA_OOM_Init;
													#if	defined (USE_MC_DEBUG) && defined (PRINTF_DEBUG)		
																	printf("\r\n  WAITDATABUF[1] finished ,the waitdata type is %2x\r\n",WAITDATABUF[1]->command_type);
													#endif
												}
												/***********等待时限内没收到想要的数据，那就继续去做其他事情*******************************************/
											else
												{tastate=TsakSW_Analy;}
										}
									/************************已经不在接收时限内了,清除等待数据的标志，同时跳转备用行动步骤********************************/
									else
										{
											WAITDATABUF[1]->WaitDataFlag=0;
											Com_Param=WAITDATABUF[1]->comparam_copy;
											Aim_Param=WAITDATABUF[1]->aimparam_copy;
											step_num=WAITDATABUF[1]->step_num_copy+4;
											
											/*****************已经产生的行动步骤不能被3注册区产生的行动步骤破坏掉*******************/
														bufthree_disable=1;
														/************************************************/
											GoToNextStep();
											
											#if	defined (USE_MC_DEBUG) && defined (PRINTF_DEBUG)		
																	printf("\r\n  WAITDATABUF[1] cancel ,the waitdata type is %2x\r\n",WAITDATABUF[1]->command_type);
													#endif
										}
							
							}
				}

/********************************************************************/
			else if(WAITDATABUF[2]->WaitDataFlag > 0 && bufthree_disable == 0)
				{
					waitData_Trigerednum=2;
						if(WAITDATABUF[2]->WaitDataFlag == 2)//需要等待立即响应
							{
									ImeAckOk_Flag=WaitIme_Ack(GetImmediaAck_Flag,ImeAck_Direction,ImeAckType,
																							WAITDATABUF[2]->command_direction,
																							WAITDATABUF[2]->command_type);
								/************************还处在接收时限内********************************/
									if(WAITDATABUF[2]->waitdata_timelimit > waitdata_time[2] ) 
									{
										/*********************如果等到了立即响应就把WaitDataFlag置1，继续等待数据*********************************/
										if(ImeAckOk_Flag == 1)  
											{
													WAITDATABUF[2]->WaitDataFlag=1;
													WAITDATABUF[2]->waitdata_timelimit=300;//300代表等待数据的时间可以是很长，大概15秒
											}
											/***********等待时限内没收到想要的数据，那就继续去做其他事情*******************************************/
										else
											{tastate=TsakSW_Analy;}
									}
									/************************已经不在接收时限内了,清除等待数据的标志，同时跳转备用行动步骤********************************/
									else
									{
										WAITDATABUF[2]->WaitDataFlag=0;
										Com_Param=WAITDATABUF[2]->comparam_copy;
										Aim_Param=WAITDATABUF[2]->aimparam_copy;
										step_num=WAITDATABUF[2]->step_num_copy+4;
										GoToNextStep();
										#if	defined (USE_MC_DEBUG) && defined (PRINTF_DEBUG)		
																	printf("\r\n no ime ack, WAITDATABUF[2] cancel ,the waitdata type is %2x\r\n",WAITDATABUF[2]->command_type);
													#endif
									}
							}
						else if(WAITDATABUF[2]->WaitDataFlag == 1 && WAITDATABUF[2]->noneed_dataresponse == 1) //不需要等待数据
							{
							/*****************清空等待数据的标志，然后继续去做其他事情***********************/
								WAITDATABUF[2]->WaitDataFlag=0;
								tastate=TsakSW_Analy;
													#if	defined (USE_MC_DEBUG) && defined (PRINTF_DEBUG)		
																	printf("\r\n  WAITDATABUF[2] finished ,the waitdata type is %2x\r\n",WAITDATABUF[2]->command_type);
													#endif
							}
						else if(WAITDATABUF[2]->WaitDataFlag == 1 && WAITDATABUF[2]->noneed_dataresponse == 2) //可能需要等待失联报告
							{
									DataAckOk_Flag=WaitData_Ack(DATABUF[0],WAITDATABUF[2]->command_direction,0x88);
										/************************还处在接收时限内********************************/
									if(WAITDATABUF[2]->waitdata_timelimit > waitdata_time[2]) 
										{
											/*********************如果等到了想要的数据就用数据触发新的行动步骤，同时清除等待标志*********************************/
											if(DataAckOk_Flag < 3 )  
												{
														WAITDATABUF[2]->WaitDataFlag=0;
														
														DataPeriod=1;
														DatabufNum=DataAckOk_Flag;
													
														tastate=TA_OOM_Init;
														#if	defined (USE_MC_DEBUG) && defined (PRINTF_DEBUG)		
																	printf("\r\n  WAITDATABUF[2] finished ,the waitdata type is %2x\r\n",0x88);
													#endif
												}
												/***********等待时限内没收到想要的数据，那就继续去做其他事情*******************************************/
											else
												{tastate=TsakSW_Analy;}
										}
									/************************已经不在接收时限内了,清除等待数据的标志，然后继续做其他事情********************************/
									else
										{
											WAITDATABUF[2]->WaitDataFlag=0;
											tastate=TsakSW_Analy;
											
											#if	defined (USE_MC_DEBUG) && defined (PRINTF_DEBUG)		
																	printf("\r\n  WAITDATABUF[2] cancel ,the waitdata type is %2x\r\n",0x88);
													#endif
										}
							
							}
						else if(WAITDATABUF[2]->WaitDataFlag == 1)//需要等待数据
							{
									DataAckOk_Flag=WaitData_Ack(DATABUF[0],WAITDATABUF[2]->command_direction,WAITDATABUF[2]->command_type);
										/************************还处在接收时限内********************************/
									if(WAITDATABUF[2]->waitdata_timelimit > waitdata_time[2]) 
										{
											/*********************如果等到了想要的数据就用数据触发新的行动步骤，同时清除等待标志*********************************/
											if(DataAckOk_Flag < 3 )  
												{
														WAITDATABUF[2]->WaitDataFlag=0;
														
														DataPeriod=1;
														DatabufNum=DataAckOk_Flag;
													
														tastate=TA_OOM_Init;
														#if	defined (USE_MC_DEBUG) && defined (PRINTF_DEBUG)		
																	printf("\r\n  WAITDATABUF[2] finished ,the waitdata type is %2x\r\n",WAITDATABUF[0]->command_type);
													#endif
												}
												/***********等待时限内没收到想要的数据，那就继续去做其他事情*******************************************/
											else
												{tastate=TsakSW_Analy;}
										}
									/************************已经不在接收时限内了,清除等待数据的标志，同时跳转备用行动步骤********************************/
									else
										{
											WAITDATABUF[2]->WaitDataFlag=0;
											Com_Param=WAITDATABUF[2]->comparam_copy;
											Aim_Param=WAITDATABUF[2]->aimparam_copy;
											step_num=WAITDATABUF[2]->step_num_copy+4;
											GoToNextStep();
											
											#if	defined (USE_MC_DEBUG) && defined (PRINTF_DEBUG)		
																	printf("\r\n  WAITDATABUF[2] cancel ,the waitdata type is %2x\r\n",WAITDATABUF[2]->command_type);
													#endif
										}
							
							}
				}
				else
					{tastate=TsakSW_Analy;}

}


/***********************************************************************
在分析等待数据的过程中需要等待立即响应的时候调用该函数

*********************************************************************/
void ImeAck_Handle(uint8_t wdr_bufnum)
{
		ImeAckOk_Flag=WaitIme_Ack(GetImmediaAck_Flag,ImeAck_Direction,ImeAckType,
																							WAITDATABUF[wdr_bufnum]->command_direction,
																							WAITDATABUF[wdr_bufnum]->command_type);
			/************************还处在接收时限内********************************/
				if(WAITDATABUF[wdr_bufnum]->waitdata_timelimit > waitdata_time[wdr_bufnum] ) 
				{
					/*********************如果等到了立即响应就把WaitDataFlag置1，继续等待数据*********************************/
					if(ImeAckOk_Flag == 1)  
						{
								WAITDATABUF[wdr_bufnum]->WaitDataFlag=1;
								WAITDATABUF[wdr_bufnum]->waitdata_timelimit=300;//300代表等待数据的时间可以是很长，大概15秒
						}
						/***********等待时限内没收到想要的数据，那就继续去做其他事情*******************************************/
					else
						{tastate=TsakSW_Analy;}
				}
				/************************已经不在接收时限内了,清除等待数据的标志，同时跳转备用行动步骤********************************/
				else
				{
					WAITDATABUF[wdr_bufnum]->WaitDataFlag=0;
					Com_Param=WAITDATABUF[wdr_bufnum]->comparam_copy;
					Aim_Param=WAITDATABUF[wdr_bufnum]->aimparam_copy;
					step_num=WAITDATABUF[wdr_bufnum]->step_num_copy+4;
					
					/*****************已经产生的行动步骤不能被其他注册区产生的行动步骤破坏掉*******************/
						WaitDataAnalysisOver_Flag=1;
					/************************************************/
					GoToNextStep();
					
								#if	defined (USE_MC_DEBUG) && defined (PRINTF_DEBUG)		
												printf("\r\n no ime ack, WAITDATABUF[%d] cancel ,the waitdata type is %2x\r\n",wdr_bufnum,WAITDATABUF[wdr_bufnum]->command_type);
								#endif
				}
}
/*****************清空等待数据的标志，然后继续去做其他事情***********************/
void WaitDataRegister_Clear(uint8_t wdr_bufnum)
{
	
	WAITDATABUF[wdr_bufnum]->WaitDataFlag=0;
	tastate=TsakSW_Analy;
						#if	defined (USE_MC_DEBUG) && defined (PRINTF_DEBUG)		
										printf("\r\n  WAITDATABUF[%d] finished ,the waitdata type is %2x\r\n",wdr_bufnum,WAITDATABUF[wdr_bufnum]->command_type);
						#endif

}

void Terminal_LoseReport_Handle(uint8_t wdr_bufnum)
{
	DataAckOk_Flag=WaitData_Ack(DATABUF[0],WAITDATABUF[wdr_bufnum]->command_direction,WAITDATABUF[wdr_bufnum]->command_type);
			/************************还处在接收时限内********************************/
		if(WAITDATABUF[wdr_bufnum]->waitdata_timelimit > waitdata_time[wdr_bufnum]) 
			{
				/*********************如果等到了想要的数据就用数据触发新的行动步骤，同时清除等待标志*********************************/
				if(DataAckOk_Flag < 3 && DATABUF[DataAckOk_Flag]->datatype == 0x88)  
					{
							WAITDATABUF[wdr_bufnum]->WaitDataFlag=0;
							
							DataPeriod=1;
							DatabufNum=DataAckOk_Flag;
						
							tastate=TA_OOM_Init;
							/*****************已经产生的行动步骤不能被其他注册区产生的行动步骤破坏掉*******************/
							WaitDataAnalysisOver_Flag=1;
							/************************************************/
							#if	defined (USE_MC_DEBUG) && defined (PRINTF_DEBUG)		
										printf("\r\n  WAITDATABUF[%d] finished ,the waitdata type is %2x\r\n",wdr_bufnum,0x88);
						#endif
					}
					/***********等待时限内没收到想要的数据，那就继续去做其他事情*******************************************/
				else
					{tastate=TsakSW_Analy;}
			}
		/************************已经不在接收时限内了,清除等待数据的标志，然后继续做其他事情********************************/
		else
			{
				WAITDATABUF[wdr_bufnum]->WaitDataFlag=0;
				tastate=TsakSW_Analy;
				
				#if	defined (USE_MC_DEBUG) && defined (PRINTF_DEBUG)		
										printf("\r\n  WAITDATABUF[%d] cancel ,the waitdata type is %2x\r\n",wdr_bufnum,0x88);
						#endif
			}
}

void WaitOneData_Handle(uint8_t wdr_bufnum,uint8_t accept_lose_report)//accept_lose_report为0时代表不接受失联报告
{
		DataAckOk_Flag=WaitData_Ack(DATABUF[0],WAITDATABUF[wdr_bufnum]->command_direction,WAITDATABUF[wdr_bufnum]->command_type);
			/************************还处在接收时限内********************************/
		if(WAITDATABUF[wdr_bufnum]->waitdata_timelimit > waitdata_time[wdr_bufnum]) 
			{
				/*********************如果等到了想要的数据就用数据触发新的行动步骤，同时清除等待标志*********************************/
				if(DataAckOk_Flag < 3 && (DATABUF[DataAckOk_Flag]->datatype != 0x88 || accept_lose_report == 1))  
					{
							WAITDATABUF[wdr_bufnum]->WaitDataFlag=0;
							
							DataPeriod=1;
							DatabufNum=DataAckOk_Flag;
						
							/*****************已经产生的行动步骤不能被其他注册区产生的行动步骤破坏掉*******************/
						WaitDataAnalysisOver_Flag=1;
					/************************************************/

							tastate=TA_OOM_Init;
						#if	defined (USE_MC_DEBUG) && defined (PRINTF_DEBUG)		
										printf("\r\n  WAITDATABUF[%d] finished ,the waitdata type is %2x\r\n",wdr_bufnum,WAITDATABUF[wdr_bufnum]->command_type);
						#endif
					}
					/***********等待时限内没收到想要的数据，那就继续去做其他事情*******************************************/
				else
					{tastate=TsakSW_Analy;}
			}
		/************************已经不在接收时限内了,清除等待数据的标志，同时跳转备用行动步骤********************************/
		else
			{
				WAITDATABUF[wdr_bufnum]->WaitDataFlag=0;
				Com_Param=WAITDATABUF[wdr_bufnum]->comparam_copy;
				Aim_Param=WAITDATABUF[wdr_bufnum]->aimparam_copy;
				step_num=WAITDATABUF[wdr_bufnum]->step_num_copy+4;
				
				/*****************已经产生的行动步骤不能被其他注册区产生的行动步骤破坏掉*******************/
						WaitDataAnalysisOver_Flag=1;
					/************************************************/
				GoToNextStep();
				#if	defined (USE_MC_DEBUG) && defined (PRINTF_DEBUG)		
										printf("\r\n  WAITDATABUF[%d] cancel ,the waitdata type is %2x\r\n",wdr_bufnum,WAITDATABUF[wdr_bufnum]->command_type);
						#endif
			}

}


void WaitMoreThanOneData_Handle(uint8_t wdr_bufnum,uint8_t accept_lose_report)
{
		DataAckOk_Flag=WaitData_Ack(DATABUF[0],WAITDATABUF[wdr_bufnum]->command_direction,WAITDATABUF[wdr_bufnum]->command_type);
			/************************还处在接收时限内********************************/
		if(WAITDATABUF[wdr_bufnum]->waitdata_timelimit > waitdata_time[wdr_bufnum]) 
			{
				/*********************如果等到了想要的数据就用数据触发新的行动步骤，*********************************/
				if(DataAckOk_Flag < 3 && (DATABUF[DataAckOk_Flag]->datatype != 0x88 || accept_lose_report == 1))  
					{
							(WAITDATABUF[wdr_bufnum]->WaitDataResponse_Quantity)--;
							waitdata_time[wdr_bufnum]=0;//等待时间累加计数器清0，以便等待下一次数据
							DataPeriod=1;
							DatabufNum=DataAckOk_Flag;
						
							if(DATABUF[DataAckOk_Flag]->datatype == 0x88) 
								{
									WAITDATABUF[wdr_bufnum]->WaitDataFlag=0;
											#if	defined (USE_MC_DEBUG) && defined (PRINTF_DEBUG)		
												printf("\r\n  WAITDATABUF[%d] cancel ,the waitdata type is %2x\r\n",wdr_bufnum,WAITDATABUF[wdr_bufnum]->command_type);
											#endif
								}
						
							/*****************已经产生的行动步骤不能被其他注册区产生的行动步骤破坏掉*******************/
						WaitDataAnalysisOver_Flag=1;
					/************************************************/

							tastate=TA_OOM_Init;
						#if	defined (USE_MC_DEBUG) && defined (PRINTF_DEBUG)		
										printf("\r\n  WAITDATABUF[%d] done a little bit ,the waitdata type is %2x,the supposed receive data quantity is %d\r\n"
													,wdr_bufnum,WAITDATABUF[wdr_bufnum]->command_type,WAITDATABUF[wdr_bufnum]->WaitDataResponse_Quantity);
						#endif
					}
					/***********等待时限内没收到想要的数据，那就继续去做其他事情*******************************************/
				else
					{tastate=TsakSW_Analy;}
			}
		/************************已经不在接收时限内了,清除等待数据的标志，同时跳转备用行动步骤********************************/
		else
			{
						#if	defined (USE_MC_DEBUG) && defined (PRINTF_DEBUG)		
							printf("\r\n  time out ,MCU have been waiting for %d timer2 interrupt \r\n",waitdata_time[wdr_bufnum]);
						#endif
				WAITDATABUF[wdr_bufnum]->WaitDataFlag=0;
				Com_Param=WAITDATABUF[wdr_bufnum]->comparam_copy;
				Aim_Param=WAITDATABUF[wdr_bufnum]->aimparam_copy;
				step_num=WAITDATABUF[wdr_bufnum]->step_num_copy+4;
				
				/*****************已经产生的行动步骤不能被其他注册区产生的行动步骤破坏掉*******************/
						WaitDataAnalysisOver_Flag=1;
					/************************************************/
				GoToNextStep();
				#if	defined (USE_MC_DEBUG) && defined (PRINTF_DEBUG)		
										printf("\r\n  WAITDATABUF[%d] cancel ,the waitdata type is %2x\r\n",wdr_bufnum,WAITDATABUF[wdr_bufnum]->command_type);
						#endif
			}

}

void NewWaitData_Analysis(void)
{
	uint8_t i;
	WaitDataAnalysisOver_Flag=0;
	for(i=0;i<3;)
	{		
			waitData_Trigerednum=i;
			if(WAITDATABUF[i]->WaitDataFlag == 2)//需要等待立即响应
				{ImeAck_Handle(i);}
			else if(WAITDATABUF[i]->WaitDataFlag == 1) 
				{
					 if(WAITDATABUF[i]->noneed_dataresponse == 1)//不需要等待数据
						{WaitDataRegister_Clear(i);}
					 else if(WAITDATABUF[i]->noneed_dataresponse == 2) //不需要等待数据但是可能需要等待失联报告
						{Terminal_LoseReport_Handle(i);}
					 else if (WAITDATABUF[i]->noneed_dataresponse == 3)//需要等待数据或者失联报告
						{
							if(WAITDATABUF[i]->WaitDataResponse_Quantity >= 2 )//需要等待多个数据或者一个失联报告
								{
									WAITDATABUF[i]->waitdata_timelimit=300;//考虑到频繁发数据无线串口忙不过来，以及目的终端失联的情况，这里把等待时限从2秒升高到15秒
									WaitMoreThanOneData_Handle(i,1);
								}
								else //此时WaitDataResponse_Quantity只可能等于1.需要等待一个数据或者一个失联报告
								{WaitOneData_Handle(i,1);}	
						}
					 else //需要等待数据，此时noneed_dataresponse 只可能等于0
						{
								if(WAITDATABUF[i]->WaitDataResponse_Quantity >= 2 )//需要等待多个数据
								{
									WAITDATABUF[i]->waitdata_timelimit=100;//考虑到频繁发数据无线串口忙不过来，这里把等待时限从2秒升高到5秒
									WaitMoreThanOneData_Handle(i,0);
								}
								else //此时WaitDataResponse_Quantity只可能等于1.
								{WaitOneData_Handle(i,0);}
						}
				}
			else {tastate=TsakSW_Analy;}
			
			if(WaitDataAnalysisOver_Flag==0) {i++;}
			else {i=4;}//用于直接跳出FOR循环
	
	}
}

/*********************************************************
从特定地址开始向前读取flash中的特定数量的数据。
因为这个函数的执行时间取决于指令中的时间跨度，所以在函数中加入喂看门狗的操作。
*********************************************************/
uint8_t LoadFlashData_And_TransUp(uint32_t addr_two,uint16_t *flash_param_buf)
{
	uint16_t datapack_num1,section_num,datapack_num2;
	uint32_t the_earlist_addr=0,addr_two_copy;
	uint16_t i,j,addr_in_certain_section;
			

 
	 datapack_num1= 		flash_param_buf[0];
	 section_num=   		flash_param_buf[1];
	 datapack_num2= 		flash_param_buf[2];
	 the_earlist_addr = (the_earlist_addr|flash_param_buf[3])<<16;
	 the_earlist_addr =  the_earlist_addr | flash_param_buf[4];
			
	 //到此处，the_earlist_addr前面的一个扇区已经被擦除重新写入了。
			
	 addr_two_copy=addr_two;
	
	 if(datapack_num1 == 0) //不可能有这种情况
		{
												 #if	defined (USE_MC_DEBUG) && defined (PRINTF_DEBUG)		
														printf("\r\n something wrong ! datapack_num1=0 \r\n");
													#endif
			return 0;
		}
	 else
		{  
		   for(i=0;i<datapack_num1;i++)//datapack_num1的取值只可能是：1,2,3.......PackNum_In_Section
				{
					addr_in_certain_section=addr_two_copy-FlashPackData_Length*i;
				  df_read_open(addr_in_certain_section);
	        df_read(&SendBufThree[8],FlashPackData_Length);
				  Usart_send(SendBufThree,FlashPackData_Length+10,COMMANDBUF[ComBufNum]->com_from,1,1);
													#if	defined (USE_MC_DEBUG) && defined (PRINTF_DEBUG)		
														printf("\r\n the data pack from addr %d transup \r\n",addr_in_certain_section);
													#endif
					
					if(addr_in_certain_section == the_earlist_addr)
					{
						i=datapack_num1;//相当于break;用于跳出当前for循环
						section_num=0;//这样后面的for循环也不会进入
						datapack_num2=0;//这样后面的for循环也不会进入
					}
					if(addr_in_certain_section % 4096 == 0)
					{
					  i=datapack_num1;//相当于break;用于跳出当前for循环
					}
				}
				
				addr_two_copy=addr_in_certain_section;
				IWDG_Feed();
				//此时section_num的取值只可能是：0,1,2......FlashValidData_MaxSectionNum
				//此时addr_two_copy的取值只可能是section_num*4096.
				
				
				for(i=0;i<section_num;i++)
					{
						if(addr_two_copy<4096)
							{addr_two_copy=(FlashValidData_MaxSectionNum<<12);}
						else
							{addr_two_copy=addr_two_copy-4096;}
							
							 for(j=0;j<PackNum_In_Section;j++)
							 {
								  addr_in_certain_section=addr_two_copy+(PackNum_In_Section-1-j)*FlashPackData_Length;
							    df_read_open(addr_in_certain_section);
							    df_read(&SendBufThree[8],FlashPackData_Length);
								  Usart_send(SendBufThree,FlashPackData_Length+10,COMMANDBUF[ComBufNum]->com_from,1,1);
													#if	defined (USE_MC_DEBUG) && defined (PRINTF_DEBUG)		
														printf("\r\n the data pack from addr %d transup \r\n",addr_in_certain_section);
													#endif
////////////								  if(addr_in_certain_section == the_earlist_addr)//已经到达了最早数据，该结束了
////////////									{
////////////									   j=PackNum_In_Section;//相当于break;跳出当前for循环
////////////										 i=section_num;//相当于break;跳出外层for循环
////////////										 datapack_num2=0;//这样后面的for循环也不会进入
////////////									}
							 }	
							 
							 if(addr_two_copy == the_earlist_addr)
							 {
							   i=section_num;//相当于break;跳出for循环
								 datapack_num2=0;//这样后面的for循环也不会进入
							 }
					IWDG_Feed();
					}

					
				//此时addr_two_copy的取值只可能是section_num*4096.
				if(addr_two_copy >=4096 ) 
					{addr_two_copy=addr_two_copy-4096;}
				else //addr_two_copy=0;
					{addr_two_copy=(FlashValidData_MaxSectionNum<<12);}
				for(i=0;i<datapack_num2;i++)//datapack_num2的取值只可能是：1,2,3.......PackNum_In_Section
					{
							addr_in_certain_section=addr_two_copy+(PackNum_In_Section-1-i)*FlashPackData_Length;
					    df_read_open(addr_in_certain_section);
							df_read(&SendBufThree[8],FlashPackData_Length);
							Usart_send(SendBufThree,FlashPackData_Length+10,COMMANDBUF[ComBufNum]->com_from,1,1);
													#if	defined (USE_MC_DEBUG) && defined (PRINTF_DEBUG)		
														printf("\r\n the data pack from addr %d transup \r\n",addr_in_certain_section);
													#endif
						  if(addr_in_certain_section == the_earlist_addr)//已经到达了最早数据，该结束了
									{
									   i=datapack_num2;//相当于break;跳出当前for循环
									}
					}
			IWDG_Feed();
			return 1;
		}

	
	
}



/*********************************************************
从特定地址开始向后读取flash中的特定数量的数据。
因为这个函数的执行时间取决于指令中的时间跨度，所以在函数中加入喂看门狗的操作。
*********************************************************/
uint8_t New_LoadFlashData_And_TransUp(uint32_t addr_two,uint16_t *flash_param_buf)
{
	uint16_t datapack_num1,section_num,datapack_num2,datapack_num1_copy=0,section_num_copy=0,datapack_num2_copy=0;
	uint32_t the_earlist_addr=0,addr_two_copy,addr_in_certain_section,addr_one;
	uint16_t i,j;
			

 
	 datapack_num1= 		flash_param_buf[0];
	 section_num=   		flash_param_buf[1];
	 datapack_num2= 		flash_param_buf[2];
	 the_earlist_addr = (the_earlist_addr|flash_param_buf[3])<<16;
	 the_earlist_addr =  the_earlist_addr | flash_param_buf[4];
			
	 //到此处，the_earlist_addr前面的一个扇区已经被擦除重新写入了。
			
	 addr_two_copy=addr_two;
	
	 if(datapack_num1 == 0) //不可能有这种情况
		{
												 #if	defined (USE_MC_DEBUG) && defined (PRINTF_DEBUG)		
														printf("\r\n something wrong ! datapack_num1=0 \r\n");
													#endif
			return 0;
		}
	 else
		{  
		   for(i=0;i<datapack_num1;i++)//datapack_num1的取值只可能是：1,2,3.......PackNum_In_Section
				{
					addr_in_certain_section=addr_two_copy-FlashPackData_Length*i;
					datapack_num1_copy=i+1;
					addr_one=addr_in_certain_section;
					if(addr_in_certain_section == the_earlist_addr)
					{
						addr_one=the_earlist_addr;
						section_num_copy=0;
						datapack_num2_copy=0;
						
						i=datapack_num1;//相当于break;用于跳出当前for循环
						section_num=0;//这样后面的for循环也不会进入
						datapack_num2=0;//这样后面的for循环也不会进入
					}
					if(addr_in_certain_section % 4096 == 0)
					{
					  i=datapack_num1;//相当于break;用于跳出当前for循环
					}
					
				}
				
				addr_two_copy=addr_in_certain_section;
				
				
				for(i=0;i<section_num;i++)
					{
						if(addr_two_copy<4096)
							{addr_two_copy=(FlashValidData_MaxSectionNum<<12);}
						else
							{addr_two_copy=addr_two_copy-4096;}
							
						section_num_copy=i+1;
							
							 for(j=0;j<PackNum_In_Section;j++)
							 {
								  addr_in_certain_section=addr_two_copy+(PackNum_In_Section-1-j)*FlashPackData_Length;
                  addr_one=addr_in_certain_section;
								  if(addr_in_certain_section == the_earlist_addr)//已经到达了最早数据，该结束了
									{
										 section_num_copy=section_num_copy-1;
										 datapack_num2_copy=j+1;
										 addr_one=the_earlist_addr;
									   j=PackNum_In_Section;//相当于break;跳出当前for循环
										 i=section_num;//相当于break;跳出外层for循环
										 datapack_num2=0;//这样后面的for循环也不会进入
									}
							 }	
							 
					}

					
				//此时addr_two_copy的取值只可能是section_num*4096.
				if(addr_two_copy >=4096 ) 
					{addr_two_copy=addr_two_copy-4096;}
				else //addr_two_copy=0;
					{addr_two_copy=(FlashValidData_MaxSectionNum<<12);}
				for(i=0;i<datapack_num2;i++)//datapack_num2的取值只可能是：1,2,3.......PackNum_In_Section
					{
							addr_in_certain_section=addr_two_copy+(PackNum_In_Section-1-i)*FlashPackData_Length;
						  addr_one=addr_in_certain_section;
						  datapack_num2_copy=i+1;
						
						  if(addr_in_certain_section == the_earlist_addr)//已经到达了最早数据，该结束了
									{
										 
										 addr_one=the_earlist_addr;
									   i=datapack_num2;//相当于break;跳出当前for循环
									}
					}

	      IWDG_Feed(); 	
				addr_one=Flashaddr_Calibration(addr_one,0);//正常情况下addr_one是准确地址不需要这个校准
/********************************************************************************************
接下来以addr_one为起点，分别向后取datapack_num2_copy个数据包，section_num_copy个扇区，datapack_num1_copy个数据包的数据
*************************************************************************************/
							#if	defined (USE_MC_DEBUG) && defined (PRINTF_DEBUG)		
								printf("\r\n get flash data from addr %d ,%d datapack,%d section,%d datapack backwards\r\n"
								,addr_one,datapack_num2_copy,section_num_copy,datapack_num1_copy);
							#endif		
					
			for(i=0;i<datapack_num2_copy;i++)//datapack_num2_copy的取值只可能是：1,2,3.......PackNum_In_Section
					{
						addr_in_certain_section=addr_one+FlashPackData_Length*i;
						df_read_open(addr_in_certain_section);
						df_read(&SendBufThree[8],FlashPackData_Length);
						Usart_send(SendBufThree,FlashPackData_Length+10,COMMANDBUF[ComBufNum]->com_from,1,1);
														#if	defined (USE_MC_DEBUG) && defined (PRINTF_DEBUG)		
															printf("\r\n the data pack from addr %d transup \r\n",addr_in_certain_section);
														#endif			
						FlashDataTransUp_Delay;//为了防止上位机粘包
					}
					
			addr_one=addr_one-(addr_one%4096);
					
			if(datapack_num2_copy == 0 && section_num_copy > 0) //addr_one所在扇区数据还未上传，要把addr_one向前移动一个扇区再进入下一个扇区发送的for 循环
				{
					if(addr_one < 4096) {addr_one = (FlashValidData_MaxSectionNum<<12);}
					else {addr_one = addr_one -4096;}
				}
					
			for(i=0;i<section_num_copy;i++)
				{
					if(addr_one+4096>FlashValidData_MaxAddr)
						{addr_one =0;	}
					else
						{addr_one=addr_one+4096;}
						
						 for(j=0;j<PackNum_In_Section;j++)
						 {
								addr_in_certain_section=addr_one+j*FlashPackData_Length;
								df_read_open(addr_in_certain_section);
								df_read(&SendBufThree[8],FlashPackData_Length);
								Usart_send(SendBufThree,FlashPackData_Length+10,COMMANDBUF[ComBufNum]->com_from,1,1);
												#if	defined (USE_MC_DEBUG) && defined (PRINTF_DEBUG)		
													printf("\r\n the data pack from addr %d transup \r\n",addr_in_certain_section);
												#endif
							 FlashDataTransUp_Delay;//为了防止上位机粘包
						 }	
						IWDG_Feed(); 
				}
////////////			if(section_num_copy == 0 && datapack_num2_copy == 0) //addr_one一定是扇区起始地址，而且该扇区的数据未被读取过，这里什么也不用干
////////////				{
////////////				
////////////				}
////////////			else if (section_num_copy == 0 && datapack_num2_copy > 0)//addr_one一定是扇区起始地址，而且该扇区的数据已经被读取过了，这里要把addr_one向后跨越一个扇区
////////////				{
////////////					if(addr_one+4096>FlashValidData_MaxAddr)
////////////						{addr_one =0;	}
////////////					else
////////////						{addr_one=addr_one+4096;}
////////////				
////////////				}
////////////			else if (section_num_copy >0 && datapack_num2_copy == 0)//addr_one一定是扇区起始地址，而且该扇区的数据已经被读取过了，这里要把addr_one向后跨越一个扇区
////////////			  {
////////////					if(addr_one+4096>FlashValidData_MaxAddr)
////////////						{addr_one =0;	}
////////////					else
////////////						{addr_one=addr_one+4096;}
////////////			  }
////////////			else if (section_num_copy >0 && datapack_num2_copy > 0)//addr_one一定是扇区起始地址，而且该扇区的数据已经被读取过了，这里要把addr_one向后跨越一个扇区
////////////			  {
////////////					if(addr_one+4096>FlashValidData_MaxAddr)
////////////						{addr_one =0;	}
////////////					else
////////////						{addr_one=addr_one+4096;}
////////////			 
////////////			  }
				
			if((section_num_copy == 0 && datapack_num2_copy == 0)==0 )
			{
				if(addr_one+4096>FlashValidData_MaxAddr)
					{addr_one =0;	}
				else
					{addr_one=addr_one+4096;}
			
			}
				
			for(i=0;i<datapack_num1_copy;i++)//datapack_num1_copy的取值只可能是：1,2,3.......PackNum_In_Section
				{
						addr_in_certain_section=addr_one+i*FlashPackData_Length;
						df_read_open(addr_in_certain_section);
						df_read(&SendBufThree[8],FlashPackData_Length);
						Usart_send(SendBufThree,FlashPackData_Length+10,COMMANDBUF[ComBufNum]->com_from,1,1);
												#if	defined (USE_MC_DEBUG) && defined (PRINTF_DEBUG)		
													printf("\r\n the data pack from addr %d transup \r\n",addr_in_certain_section);
												#endif
					  FlashDataTransUp_Delay;//为了防止上位机粘包

				}
					    				
			return 1;
		}

	
	
}