#include "mul_task.h"

uint8_t task_sw_flag=0;//ÈÎÎñÇĞ»»µÄ±êÖ¾£¬ÔÚÏµÍ³¶¨Ê±Æ÷Àï£¬Ã¿¸ô10ºÁÃëÖÃ1
uint8_t ComBufNum=0,DatabufNum=0;
uint8_t DataPeriod=0;//¸Ã±êÖ¾ÖÃ1ËµÃ÷º¯ÊıMul_Task´¦ÓÚdata´¥·¢µÄÈÎÎñÖĞ


uint8_t Com_Param=0,Aim_Param=0,step_num,task_step;
uint8_t task_complete;

uint8_t SendBufOne[150]={0x66,0xcc},Comnbinedbuf[10];
extern uint8_t SendBufTwo[10];
uint8_t SendBufThree[FlashPackData_Length+20]={0x66,0xcc};//×¨ÃÅÓÃÓÚ·¢ËÍflashÖĞµÄÀúÊ·ĞÅÏ¢
uint16_t  DataBufLength;
/*******************ÓÃÓÚµÈ´ıÊı¾İ»ØÓ¦µÄ±äÁ¿***************************/
uint8_t 	wait_data_flag=0,waitdata_type;
uint16_t  wait_data_response_quantity;//µÈ´ıÍ¬ÖÖÊı¾İ»ØÓ¦µÄÊıÁ¿
extern WaitDataRegist_Typedef *WAITDATABUF[3];
extern uint16_t waitdata_time[3];//waitdata_time_one,waitdata_time_two,waitdata_time_three;
uint8_t waitdata_registnum=0;//¸Ã±äÁ¿µÈÓÚ0´ú±íÃ»ÓĞ½á¹¹Ìå±»Ê¹ÓÃ£¬1´ú±íµÚÒ»¸ö½á¹¹Ìå±»Ê¹ÓÃÁË£¬2 £¬3ÒÀ´ËÀàÍÆ
uint8_t waitData_Trigerednum=0;//¸Ã±äÁ¿±íÊ¾Òı·¢Ò»ÏµÁĞÈÎÎñµÄÊı¾İ×¢²á±íµÄ±êºÅ
uint8_t NoneedDataResponse_Flag=0;//¸Ã±äÁ¿ÖÃÒ»±íÊ¾²»ĞèÒªµÈ´ıÊı¾İ»ØÓ¦£¨µ«¿ÉÄÜĞèÒªµÈ´ıÁ¢¼´ÏìÓ¦£©
uint8_t WaitDataAnalysisOver_Flag=0;//
/*******************ÓÃÓÚ¶Ï¹ì¼ì²éµÄ±äÁ¿***************************/
uint32_t DG_checkcounter=0;
uint8_t DG_StateOne=0,DG_StateOne_faraway=0,DG_StateTwofaraway=0,DG_StateTwo=0,WaveFromFarAway=0;
uint16_t Rec_num[]={0x00},timeout_cnt=0;
uint8_t position_mul;
/**************************************************************/
////////////uint16_t test_addr=1008;
////////////extern uint8_t TrackInfCollect_Buf[1100];

/*******************ÓÃÓÚµÈ´ıÁ¢¼´ÏìÓ¦µÄ±äÁ¿***************************/
extern uint8_t GetImmediaAck_Flag,ImeAck_Direction,ImeAckType;//ÕâÁ½¸ö±äÁ¿ÔÚÊÕµ½ÍêÕûÁ¢¼´ÏìÓ¦Ö¡µ«ÊÇ»¹Ã»´æÈë»º³åÇøµÄÊ±ºò±»¸³Öµ
uint8_t ImeAckOk_Flag=0,DataAckOk_Flag=0;
uint16_t WaitDataTime;

/**************************************************************/

/*******************ÓÃÓÚ´æ´¢Ìú¹ìĞÅÏ¢µÄÊı×é***************************/
uint8_t RccInfBuf[7];													//,7¸öÔªËØ·Ö±ğ´æ´¢£ºÄêÔÂÈÕÊ±·ÖÃë£¬ÆäÖĞÄêÕ¼ÁË2¸ö×Ö½Ú£¬
																							//±ÈÈç2016ÄêµÄ16½øÖÆĞÎÊ½ÊÇ0x07e0,ÄÇÃ´Á½¸ö×Ö½Ú·Ö±ğÊÇ0x07,0xe0
uint8_t DGStateInfBuf[2];											//
uint8_t StressInfBuf[4]; 											//Á½¸ù¹ìµÄÓ¦Á¦£¬Ò»¸ùÁ½±ÈÌØ
uint8_t TemperatureInfBuf[4];									//Á½¸ù¹ìµÄÎÂ¶È£¬Ò»¸ùÁ½±ÈÌØ
uint8_t LocalSignalAmplitudeInfBuf[4];				//½ü¶ËÌú¹ìĞÅºÅ·ùÖµ
uint8_t SignalAmplitudeInfBuf[64];						//Á½¸ù¹ìµÄĞÅºÅ·ùÖµ£¬Ò»¸ù¹ì2¸ö·½Ïò£¬Ò»¸ö·½Ïò16±ÈÌØ
/**************************************************************/

uint8_t WLuart_header_addr=0;//ÓÃÓÚ´æ´¢ÎŞÏß´®¿Ú½«Òª·¢ËÍµÄÄ¿µÄÖÕ¶ËºÅ
uint8_t Launch_Announcement_Flag=0;//¸Õ¸Õ½øĞĞÁËÒ»´Î³¬ÉùĞÅºÅ·¢ÉäÍ¨¸æµÄ±êÖ¾¡£ÔÚPFGAÅäºÏCPUµÄÇé¿öÏÂ£¬¸Ã±êÖ¾Ã¿¸ô75Ãë±»ÖÃÒ»Ò»´Î


extern uint8_t datarestore_buff[660],comrestore_buff[200];
extern DataRestore_typedef *DATABUF[3];
extern ComRestore_typedef  *COMMANDBUF[3];
extern uint8_t BasicInf_recbuf[6];
extern uint16_t wait_time;
extern uint8_t   MulTrans_startaddr,MulTrans_finaladdr;//ÊÕµ½¶à²¥Ö¸Áîºó°ÑÆğµãµØÖ·ºÍÖÕµãµØÖ·´æÈëÕâÁ½¸ö±êÖ¾Àï

/***************************¿ØÖÆÉÁµÆµÄ±äÁ¿****************************/
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
/**********************¹ØÓÚRTCµÄ±äÁ¿**************************/
extern uint16_t Year;
extern uint8_t Month;
extern uint8_t Day;
extern uint8_t Hour;
extern uint8_t Minute;
extern uint8_t Second;
extern uint32_t Time_Waited_Configured;
/****************************************************************/
/***********************¹ØÓÚÓ¦Á¦µÄ±äÁ¿***********************/
uint8_t Max485_Sendflag=0;//485·¢ËÍÃüÁîµÄ±êÖ¾
extern uint8_t MAX485_Recbuf[50];
/****************************************************************/
uint8_t tx_flag1,tx_flag2;

TakeAction_typedef  tastate=TA_Idle;

/*************²Á³ı±¸·İ¼Ä´æÆ÷µÄÊ±ºòÊ¹ÓÃ¸ÃÊı×é*****************/
uint8_t BackupRegEraseSentbuf[20]={
0xFF,0xFF,0xFF,0xFF,0xFF, 
0xFF,0xFF,0xFF,0xFF,0xFF, 
0xFF,0xFF,0xFF,0xFF,0xFF, 
0xFF,0xFF,0xFF,0xFF,0xFF, 
};
/********************************************/
/***************¹ØÓÚÉÏÎ»»ú¶ÁÈ¡flashÖĞÀúÊ·ĞÅÏ¢µÄ²ÎÊı******************/
uint32_t Flash_Addr2;
uint16_t Flash_Parabuf[5];
/********************************************/
void Mul_Task(void)
{
	switch(tastate)
		{
/*************************************************************************************************
×´Ì¬±àºÅ£º3 
¹¦ÄÜ£º¼ì²éÈÎÎñÇĞ»»±êÖ¾task_sw_flag£¨¸Ã±äÁ¿ÔÚÏµÍ³Ê±ÖÓÖĞ¶ÏÀï¶¨ÆÚÖÃ1£©£¬
			Èç¹ûtask_sw_flag=1¼´´ú±íĞèÒªÈÎÎñÇĞ»»£¬½øÈëDataRegistAnaly£¬×´Ì¬±àºÅ4£¬·ñÔò¼ÌĞø´ıÔÚTA_Idle×´Ì¬
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
										if(task_sw_flag ==1)//Õâ¸ö±êÖ¾±¾ÊÇÓÃÀ´·ÅÂıÈÎÎñÇĞ»»ËÙ¶ÈµÄ£¬Èç½ñÓÃÓÚÎ¹¿´ÃÅ¹·
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
×´Ì¬±àºÅ£º4
¹¦ÄÜ£º¼ì²éµÈ´ıÊı¾İ±êÖ¾wait_data_flag£¨¸Ã±äÁ¿ÔÚ×´Ì¬WaitDataRegistÖĞÖÃ1£¬×´Ì¬±àºÅ13£©
			Èç¹û¸Ã±êÖ¾Îª0£¬ËµÃ÷MCU²»ĞèÒªµÈ´ı½ÓÊÕÊı¾İ£¬Ö±½ÓÌø×ª×´Ì¬TsakSW_Analy£¬×´Ì¬±àºÅ5
			Èç¹û¸Ã±êÖ¾Îª1£¬¾Í¼ì²éÊı¾İ½ÓÊÕ»º³åÇø£¬Èç¹û²éµ½ÁËĞèÒªµÄÊı¾İ¾ÍÌø×ª×´Ì¬TA_OOM_Init£¬×´Ì¬±àºÅ6
						Èç¹ûÃ»²éµ½£¬¾ÍÌø×ª×´Ì¬TsakSW_Analy
****************************************************************************************************/
			case  DataRegistAnaly:
//////											#if	defined (USE_MC_DEBUG) && defined (PRINTF_DEBUG)		
//////													mul_task_debug();
//////												#endif				
					NewWaitData_Analysis();
					
				break;
/*************************************************************************************************
×´Ì¬±àºÅ£º5
¹¦ÄÜ£º¼ì²éÃüÁî»º³åÇø²¢ÇÒÈ¡µÃ×î¸ßÓÅÏÈ¼¶µÄÃüÁîËù¶ÔÓ¦µÄ»º³åÇø±êºÅ£¬
			Èç¹ûÃ»ÓĞ¿ÉÖ´ĞĞµÄÃüÁî¾ÍÌø×ª×´Ì¬DGCHECKÈ¥Ö´ĞĞÄ¬ÈÏµÄ¶Ï¹ì¼ì²éÈÎÎñ
			Èç¹ûÓĞ¿ÉÖ´ĞĞÃüÁî£¬¾Í°Ñ¸ÃÃüÁîµÄ»º³åÇø±êºÅ¼ÇÂ¼ÔÚÈ«¾Ö±äÁ¿ComBufNumÖĞ£¬²¢ÇÒÌø×ªTA_OOM_Init£¬×´Ì¬±àºÅ6
****************************************************************************************************/
			case	TsakSW_Analy:
					ComBufNum=Check_ComBuf(COMMANDBUF[0]);
////											#if	defined (USE_MC_DEBUG) && defined (PRINTF_DEBUG)		
////													mul_task_debug();
////												#endif
					if(ComBufNum >=3) {tastate=DGCHECK;}//Ã»ÓĞÈÎÎñÇĞ»»µÄÊ±ºòÖ´ĞĞ¶Ï¹ì¼ì²éÈÎÎñ
					else {tastate=TA_OOM_Init;}
				break;
/*************************************************************************************************
×´Ì¬±àºÅ£º1  
¹¦ÄÜ£ºÖ´ĞĞÒ»´Î¶Ï¹ì¼ì²éÈÎÎñ£¬È»ºóÌø×ªTA_Idle×´Ì¬£¨3£©
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
×´Ì¬±àºÅ£º6
¹¦ÄÜ£º»ñÈ¡ĞĞ¶¯²½Öè£¬¾ßÌåÀ´Ëµ¾ÍÊÇ£º
			¸ù¾İDataPeriodÀ´ÅĞ¶Ï±¾´ÎĞĞ¶¯²½ÖèÊÇÊı¾İ´¥·¢µÄ»¹ÊÇÖ¸Áî´¥·¢µÄ¡£
			Èç¹ûDataPeriod=0£¬¾Í´ÓÖ¸Áî»º³åÇøÖĞÈ¡µÃÖ¸ÁîÀàĞÍºÍÖ¸ÁîÄ¿±ê×é³É»ñÈ¡ĞĞ¶¯²½ÖèµÄ±ØÒª²ÎÊı
			Èç¹ûDataPeriod=1£¬¾Í´ÓÊı¾İ»º³åÇøÖĞÈ¡µÃÊı¾İÀàĞÍºÍÊı¾İÄ¿±ê×é³É»ñÈ¡ĞĞ¶¯²½ÖèµÄ±ØÒª²ÎÊı
			
			Èç¹û»ñÈ¡±ØÒª²ÎÊıÊ§°Ü£¬¾ÍÌø×ªTA_Idle×´Ì¬£¬»ñÈ¡³É¹¦£¬¾ÍÌø×ªTA_TaskBegin£¬×´Ì¬±àºÅ7
****************************************************************************************************/
			case TA_OOM_Init:
//											#if	defined (USE_MC_DEBUG) && defined (PRINTF_DEBUG)		
//													mul_task_debug();
//												#endif
					if(OOM_Init(ComBufNum,DataPeriod,DatabufNum)==1){tastate=TA_TaskBegin;}
					else {tastate=THEEND;}//ÍòÒ»ÊÕµ½ÁËÎ´¶¨ÒåµÄÖ¸Áî»òÊı¾İ£¬Òªµ½THEEND×´Ì¬°Ñ¸ÃÖ¸Áî»òÊı¾İµÄ»º³åÇøÖÃÎªÒÑÊ¹ÓÃ¹ı
				break;
/*************************************************************************************************
×´Ì¬±àºÅ£º7
¹¦ÄÜ£º°ÑĞĞ¶¯²½Öè¼ÆÊı±äÁ¿step_numÇåÁã£¬È»ºóÌø×ªµÚ0ºÅĞĞ¶¯²½ÖèËù´ú±íµÄ×´Ì¬£¬È»ºó°ÑĞĞ¶¯²½Öè¼ÆÊı±äÁ¿+1£¬ÒÔ±ãÏÂÒ»´ÎÌø×ª
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
×´Ì¬±àºÅ£º2
¹¦ÄÜ£º
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
					/*********************²âÊÔflash********************/
				
////////					df_write_open(test_addr);
////////					df_write(TrackInfCollect_Buf,256);
////////					test_addr=test_addr+256;
			    /******************************************************************/
					GoToNextStep();
				break;
/*************************************************************************************************
×´Ì¬±àºÅ£º0
¹¦ÄÜ£ºÒ»´ÎÖ¸Áî»òÕßÊı¾İ´¥·¢µÄÒ»ÏµÁĞĞĞ¶¯²½Öè×ßµ½ÁË×îºóÒ»²½£¬ÇåÀíÒ»Ğ©±äÁ¿ºÍ±êÖ¾¡£
			Èç¹ûDataPeriod=1£¬´ú±í±¾´ÎĞĞ¶¯²½ÖèÊÇÊı¾İ´¥·¢µÄ£¬¾Í°ÑDataPeriod£¬wait_data_flag£¬ºÍÊı¾İµÄdata_used±êÖ¾Çå³ıµô¡
			Èç¹ûÊÇÖ¸Áî´¥·¢ÁË±¾ÂÖĞĞ¶¯£¬¾Í°ÑÖ¸ÁîËù¶ÔÓ¦µÄcom_usedÖÃ1.
****************************************************************************************************/
			case THEEND:
											#if	defined (USE_MC_DEBUG) && defined (PRINTF_DEBUG)		
													mul_task_debug();
												#endif
					if(DataPeriod ==1 )
						{
							DataPeriod=0;
							wait_data_flag=0;
							/********************Ïàµ±ÓÚÇå¿Õ½ÓÊÕ»º³åÇø**********************/
								DATABUF[DatabufNum]->data_used=1;
						}
					else
						{
						COMMANDBUF[ComBufNum]->com_used=1;//Ö¸ÁîÒÑ¾­±»ÓÃ¹ıÁË£¬¸Ã»º³åÇø¿ÉÒÔ±»ĞÂÊÕµ½µÄÖ¸Áî¸²¸ÇÁË	
						}
					NoneedDataResponse_Flag=0;
					task_complete=0;
					tastate=TA_Idle;
				break;
/*************************************************************************************************
×´Ì¬±àºÅ£º8
****************************************************************************************************/
			case LoadTerminalBasicInf:
											#if	defined (USE_MC_DEBUG) && defined (PRINTF_DEBUG)		
													mul_task_debug();
												#endif
					DataBufLength=15;
					task_complete=LoadBasicInf(DataBufLength);
						
					/**********************FLASHµ÷ÊÔ£¬N25Q128A*************************/
//									#if	defined (USE_MC_DEBUG) && defined (PRINTF_DEBUG)		
////											printf("\r\n SPI_FLASH_Test \r\n");
////											SPI_FLASH_Test();
////												FLASH_ReadWrite_Test();
//									#endif
					/**********************************************************/
					GoToNextStep();
				break;
/*************************************************************************************************
×´Ì¬±àºÅ£º9
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
×´Ì¬±àºÅ£º16
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
×´Ì¬±àºÅ£º10
****************************************************************************************************/
			case TransUpSendbufOne:
											#if	defined (USE_MC_DEBUG) && defined (PRINTF_DEBUG)		
													mul_task_debug();
												#endif
						SendBufOne[0]=0x66;SendBufOne[1]=0xcc;
					if(step_num < 8)
						{
							Usart_send(SendBufOne,DataBufLength,COMMANDBUF[ComBufNum]->com_from,1,1);//ĞèÒª¼ÆËãĞ£ÑéºÍ£¬Êı¾İ×°ÔØ2Î»Ğ£ÑéºÍ
						}
					else
						{
							Usart_send(SendBufOne,DataBufLength,WAITDATABUF[waitData_Trigerednum]->command_comfrom,1,1);//ĞèÒª¼ÆËãĞ£ÑéºÍ£¬Êı¾İ×°ÔØ2Î»Ğ£ÑéºÍ
						}
			
					GoToNextStep();
				break;
/*************************************************************************************************
×´Ì¬±àºÅ£º11
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
×´Ì¬±àºÅ£º12
Ò»µ©×ß½øÕâ¸ö×´Ì¬£¬ËµÃ÷Ö¸ÁîÊÇÅäÖÃÖ¸Áî£¬NoneedDataResponse_FlagÒªÃ´ÊÇ1£¬ÒªÃ´ÊÇ2¡£
1´ú±íÍêÈ«²»ĞèÒªµÈ´ıÊı¾İ»ØÓ¦£¬µ±Ö¸ÁîµÄÄ¿µÄÖÕ¶ËÓë±¾¶ËÏàÁÚµÄÊ±ºò£¬
					ÍêÈ«²»ĞèÒªµÈ´ıÊı¾İ»ØÓ¦£¬Ö»ĞèµÈ´ıÁ¢¼´ÏìÓ¦¼´¿ÉÅĞ¶ÏÁÙ¶ËÊÇ·ñÊ§Áª
2´ú±í»òĞíĞèÒªµÈ´ıÊı¾İ»ØÓ¦¡£µ±Ö¸ÁîµÄÄ¿µÄÖÕ¶Ë·Ç±¾¶Ë£¨ÄÜ×ßµ½Õâ¸ö×´Ì¬Àï£¬Ö¸ÁîµÄÄ¿µÄÖÕ¶ËÒ»¶¨·Ç±¾¶Ë£©Ò²·ÇÁÙ¶ËµÄÊ±ºò£¬
					µÈ´ıÍêÁ¢¼´ÏìÓ¦»¹ÓĞ¿ÉÄÜĞèÒªµÈ´ıÊ§Áª±¨¸æ¡£
****************************************************************************************************/
			case NoneedDataResponse://
						
						wait_data_flag=2;//ÄÜ×ß½øÕâ¸ö×´Ì¬ÀïÒ»¶¨ĞèÒªµÈ´ıÁ¢¼´ÏìÓ¦£¬Ò»¶¨ÊÇÅäÖÃÖ¸Áî
			
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
×´Ì¬±àºÅ£º17
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
×´Ì¬±àºÅ£º13
½«ĞèÒªµÈ´ıµÄÊı¾İ»ØÓ¦µÄÀàĞÍºÍ·½Ïò¼ÇÂ¼µÈÏÂÀ´
Í¬Ê±½«Ò»µ©µÈµ½ÁËÊı¾İËùĞèµÄĞĞ¶¯²½Öè²ÎÊı¼ÇÂ¼ÏÂÀ´
****************************************************************************************************/
			case	WaitDataRegist:
			
						/*******************µÈ´ı»ØÓ¦µÄÀàĞÍÖ»¿ÉÄÜÊÇÃüÁîÀàĞÍ»òÕßÊÇ´íÎóÀàĞÍ0X88*********************************************/
						waitdata_type=COMMANDBUF[ComBufNum]->comtype;
					/***************************************************************************************************/
////////////						if(COMMANDBUF[ComBufNum]->comtype== 0xf4)//»ñÈ¡flashÖĞÒ»¶ÎÊ±¼äÄÚ´æ´¢µÄµ¥µãÌú¹ìÈ«²¿ĞÅÏ¢
////////////							{
////////////								
////////////								wait_data_response_quantity=*48;//flash_read_time_cal*3600/75
////////////							}
////////////						else
							{wait_data_response_quantity=1;}
						/********************Èç¹ûÖ¸ÁîµÄÄ¿µÄµØÖ·ÊÇÏàÁÚÖÕ¶Ë£¬¾ÍÖ»ĞèÒªµÈ´ıÊı¾İ»ØÓ¦£¬·ñÔòĞèÒªÏÈµÈ´ıÁ¢¼´ÏìÓ¦£¬ÔÙµÈ´ıÊı¾İ»ØÓ¦*******/
						/*********************Í¬Ê±ĞèÒª±ê¼ÇÖ¸ÁîµÄ·½Ïò£¬Á¢¼´ÏìÓ¦ºÍÊı¾İ»ØÓ¦µÄ·½ÏòÒ»¶¨ÓëÖ®Ïà·´********************************************************************************************/
						if(NoneedDataResponse_Flag == 0)//Ö¸ÁîÊÇ¶ÁÈ¡ĞÅÏ¢Ö¸Áî£¨Èç¹ûÊÇÅäÖÃÖ¸Áî£¬NoneedDataResponse_Flag=1»òÕßNoneedDataResponse_Flag=2£©
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
×´Ì¬±àºÅ£º14
****************************************************************************************************/
	  	case TransUpTheData:
											#if	defined (USE_MC_DEBUG) && defined (PRINTF_DEBUG)		
													mul_task_debug();
												#endif
			/********************************¸Ä±äÊı¾İÔ­µØÖ·*************************************/
						DATABUF[DatabufNum]->datacontent[4]=TerminalBasicInf->LocalTerminalAddr;
			/********************************¸Ä±äÊı¾İµÄÄ¿µÄµØÖ·************************************************************/
						if(WAITDATABUF[waitData_Trigerednum]->command_comfrom == 1) {DATABUF[DatabufNum]->datacontent[5]=0xff;}
						else if(DATABUF[DatabufNum]->data_direction == 1) //
							{DATABUF[DatabufNum]->datacontent[5]=TerminalBasicInf->NextTerminalAddr;}
						else if(DATABUF[DatabufNum]->data_direction == 2) 
							{DATABUF[DatabufNum]->datacontent[5]=TerminalBasicInf->FrontTerminalAddr;}
				/******************************·¢ËÍÊı¾İ**************************************************************/			
						Usart_send(DATABUF[DatabufNum]->datacontent,DATABUF[DatabufNum]->datalength,WAITDATABUF[waitData_Trigerednum]->command_comfrom ,1,1);
						GoToNextStep();
	  		break;
/*************************************************************************************************
×´Ì¬±àºÅ£º15
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
×´Ì¬±àºÅ£º18
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
×´Ì¬±àºÅ£º19
****************************************************************************************************/
			case LoadDGStateInf:		//19
											#if	defined (USE_MC_DEBUG) && defined (PRINTF_DEBUG)		
													mul_task_debug();
												#endif
//							LoadDGState_inf();
							GoToNextStep();
				break;
/*************************************************************************************************
×´Ì¬±àºÅ£º20
****************************************************************************************************/
			case LoadStressInf:		//20
											#if	defined (USE_MC_DEBUG) && defined (PRINTF_DEBUG)		
													mul_task_debug();
												#endif
					GoToNextStep();

							
				break;
/*************************************************************************************************
×´Ì¬±àºÅ£º21
****************************************************************************************************/
			case LoadTemperatureInf:			//21
				
							LoadTemperature_Inf();
			
											#if	defined (USE_MC_DEBUG) && defined (PRINTF_DEBUG)		
													mul_task_debug();
												#endif
							
							GoToNextStep();
				break;
/*************************************************************************************************
×´Ì¬±àºÅ£º22
****************************************************************************************************/
			case LoadSignalAmplitudeInf://22
											#if	defined (USE_MC_DEBUG) && defined (PRINTF_DEBUG)		
													mul_task_debug();
												#endif
//							LoadSignalAmplitude_Inf();
							GoToNextStep();
				break;
/*************************************************************************************************
×´Ì¬±àºÅ£º23
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
×´Ì¬±àºÅ£º24
****************************************************************************************************/
			case EraseTheFlash:		//24
											#if	defined (USE_MC_DEBUG) && defined (PRINTF_DEBUG)		
													mul_task_debug();
												#endif
							Erase_The_Flash(&(COMMANDBUF[ComBufNum]->comcontent[7]));
			
							GoToNextStep();
				break;
/*************************************************************************************************
×´Ì¬±àºÅ£º25
****************************************************************************************************/
			case SetRTC:
											
			
								Time_Waited_Configured=Time_Regulate(&(COMMANDBUF[ComBufNum]->comcontent[7]));//½«Ö¸ÁîÖĞµÄÄêÔÂÈÕÊ±·ÖÃëĞÅÏ¢¸³Öµ¸øÌØ¶¨È«¾Ö±äÁ¿£¬Í¬Ê±×ª»»³ÉRTC¼ÆÊıÖµ·µ»Ø
								if(Time_Waited_Configured !=0)//Èç¹ûÊÚÊ±·¶Î§ÕıÈ·
								{
									Time_Adjust( Time_Waited_Configured);//ĞŞ¸ÄRTCÊ±ÖÓ¼ÆÊıÆ÷
									FPGA_SendWave_Init();//
								}
								
													#if	defined (USE_MC_DEBUG) && defined (PRINTF_DEBUG)		
														mul_task_debug();
													#endif
									GoToNextStep();
				break;
/*************************************************************************************************
×´Ì¬±àºÅ£º26
****************************************************************************************************/							
			case ThresHoldSet:
													#if	defined (USE_MC_DEBUG) && defined (PRINTF_DEBUG)		
														mul_task_debug();
													#endif
							
								write(0x15,COMMANDBUF[ComBufNum]->comcontent[6]);
								write(0x16,COMMANDBUF[ComBufNum]->comcontent[7]);
							
								/*********************ÁÙÊ±µ÷ÊÔFLASH***************************/
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
×´Ì¬±àºÅ£º27
****************************************************************************************************/	
			case LoadFlashDataAndTransUp:
							IWDG_Feed();//Õâ²½²Ù×÷»áºÄ·ÑÒ»¶¨Ê±¼äËùÒÔÔÚÕâÀïÎ¹Ò»ÏÂ¿´ÃÅ¹·
			
													#if	defined (USE_MC_DEBUG) && defined (PRINTF_DEBUG)		
														mul_task_debug();
													#endif
							LoadDataBufBasicInf(SendBufThree);
			        
              task_complete=New_LoadFlashData_And_TransUp(Flash_Addr2,Flash_Parabuf);
			        
							Check_CollectInf_TransupFlag();
							/*******************·¢ËÍ´«Êä½áÊø±êÖ¾***************************/
			        FlashDataTransUp_Delay;//ÎªÁË·ÀÖ¹ÉÏÎ»»úÕ³°ü
			        SendBufThree[7]=0xff;
			        Usart_send(SendBufThree,10,COMMANDBUF[ComBufNum]->com_from,1,1);
			
							GoToNextStep();
				break;
/*************************************************************************************************
×´Ì¬±àºÅ£º28
****************************************************************************************************/
			case EraseTheBackupRegister:		
											#if	defined (USE_MC_DEBUG) && defined (PRINTF_DEBUG)		
													mul_task_debug();
												#endif
							Erase_The_BackupRegister(&(COMMANDBUF[ComBufNum]->comcontent[7]));
			
							GoToNextStep();
				break;
/*************************************************************************************************
×´Ì¬±àºÅ£º29
****************************************************************************************************/
case GetParamAboutLoadFlashInf:		
											IWDG_Feed();//Õâ²½²Ù×÷»áºÄ·ÑÒ»¶¨Ê±¼äËùÒÔÔÚÕâÀïÎ¹Ò»ÏÂ¿´ÃÅ¹·
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
									/*******************·¢ËÍ´«Êä½áÊø±êÖ¾***************************/
									SendBufThree[7]=0xff;
									Usart_send(SendBufThree,10,COMMANDBUF[ComBufNum]->com_from,1,1);
									tastate=THEEND;
								}
							else
								{GoToNextStep();}
				break;
/*************************************************************************************************
×´Ì¬±àºÅ£º30
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
	
//		/**************************½«ĞÂµÄ»ù±¾ĞÅÏ¢ÅäÖÃ´æÈëFLASH***********************************/
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
	/*********************×°ÔØÔ­µØÖ·ºÍÄ¿µÄµØÖ·******************************/
	if(COMMANDBUF[ComBufNum]->com_from ==1 )//¸ÃÖ¸ÁîÓÉPC·¢³ö
		{
				databuf[4]=TerminalBasicInf->LocalTerminalAddr;//oriaddr
				databuf[5]=0xff;//desaddr
		}
	else //¸ÃÖ¸ÁîÓÉÏàÁÚÖÕ¶Ë·¢³ö
		{
				databuf[4]=TerminalBasicInf->LocalTerminalAddr;//oriaddr¸ÄÎª±¾¶ËµØÖ·
				if(COMMANDBUF[ComBufNum]->com_direction == 1)//ÕıÏò
					{		
						databuf[5]=TerminalBasicInf->FrontTerminalAddr;
					}
				else if(COMMANDBUF[ComBufNum]->com_direction == 2)//·´Ïò
					{				
						databuf[5]=TerminalBasicInf->NextTerminalAddr;
					}
			
		}
	
	/*******************×°ÔØÖ¸ÁîÀàĞÍ*********************/
		databuf[6]=COMMANDBUF[ComBufNum]->comtype;//type
		/*******************×°ÔØ±¾¶ËµØÖ·***********************/
		databuf[7]=TerminalBasicInf->LocalTerminalAddr;
}

uint8_t LoadBasicInf(uint16_t length)
{
//	uint8_t i;
//	uint16_t sum=0;
//	SendBufOne[2]=(uint8_t)(length>>8);
//	SendBufOne[3]=(uint8_t)(length);
	
	if(COMMANDBUF[ComBufNum]->com_from ==1 )//¸ÃÖ¸ÁîÓÉPC·¢³ö
		{
				SendBufOne[4]=TerminalBasicInf->LocalTerminalAddr;//oriaddr
				SendBufOne[5]=0xff;//desaddr
		}
	else //¸ÃÖ¸ÁîÓÉÏàÁÚÖÕ¶Ë·¢³ö
		{
				SendBufOne[4]=TerminalBasicInf->LocalTerminalAddr;//oriaddr¸ÄÎª±¾¶ËµØÖ·
				if(COMMANDBUF[ComBufNum]->com_direction == 1)//ÕıÏò
					{		
						SendBufOne[5]=TerminalBasicInf->FrontTerminalAddr;
					}
				else if(COMMANDBUF[ComBufNum]->com_direction == 2)//·´Ïò
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
			if(COMMANDBUF[ComBufNum]->com_from ==1 )//¸ÃÖ¸ÁîÓÉPC·¢³ö
				{
						SendBufOne[4]=TerminalBasicInf->LocalTerminalAddr;//oriaddr
						SendBufOne[5]=0xff;//desaddr
				}
			else //¸ÃÖ¸ÁîÓÉÏàÁÚÖÕ¶Ë·¢³ö
				{
						SendBufOne[4]=TerminalBasicInf->LocalTerminalAddr;//oriaddr¸ÄÎª±¾¶ËµØÖ·
						if(COMMANDBUF[ComBufNum]->com_direction == 1)//ÕıÏò
							{		
								SendBufOne[5]=TerminalBasicInf->FrontTerminalAddr;
							}
						else if(COMMANDBUF[ComBufNum]->com_direction == 2)//·´Ïò
							{				
								SendBufOne[5]=TerminalBasicInf->NextTerminalAddr;
							}
					
				}
		
		}
	
	else //ËµÃ÷ÊÇ±¸ÓÃµÄĞĞ¶¯²½ÖèÖĞµ÷ÓÃÁËÕâ¸öº¯Êı£¬´ËÊ±COMMANDBUF[ComBufNum]»òĞí»áÊ§Ğ§
		{
				SendBufOne[6]=WAITDATABUF[waitData_Trigerednum]->command_type;//type
				if(WAITDATABUF[waitData_Trigerednum]->command_comfrom ==1 )//¸ÃÖ¸ÁîÓÉPC·¢³ö
				{
						SendBufOne[4]=TerminalBasicInf->LocalTerminalAddr;//oriaddr
						SendBufOne[5]=0xff;//desaddr
				}
			else //¸ÃÖ¸ÁîÓÉÏàÁÚÖÕ¶Ë·¢³ö
				{
						SendBufOne[4]=TerminalBasicInf->LocalTerminalAddr;//oriaddr¸ÄÎª±¾¶ËµØÖ·
						if(WAITDATABUF[waitData_Trigerednum]->command_direction == 1)//ÕıÏò
							{		
								SendBufOne[5]=TerminalBasicInf->FrontTerminalAddr;
							}
						else if(WAITDATABUF[waitData_Trigerednum]->command_direction == 2)//·´Ïò
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
	
	
	if(checksum==1)//ĞèÒª¼ÆËãĞ£ÑéºÍ
		{
			if( com_dat == 1) //data×°ÔØÁ½Î»Ğ£ÑéºÍÒÔ¼°2Î»Êı¾İ³¤¶È
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
			else //command×°ÔØÒ»Î»Ğ£ÑéºÍÒÔ¼°Ò»Î»Ö¸Áî³¤¶È
			{
				sendbuf[2]=(uint8_t)(length);
				
				for(i=0;i<length-1;i++)
					{
						sum=sum+sendbuf[i];
					}	
				sendbuf[length-1]=(uint8_t)(sum);
			}
	
		}
		
	if(unique_ID == 1) //»ØÓ¦¸øÉÏÎ»»ú
		{
			for (i=0;i<length;i++)
				{
					while(USART_GetFlagStatus(USART2, USART_FLAG_TC)==RESET);
					USART_SendData(USART2, sendbuf[i]);				
				}
		}
	else //»ØÓ¦¸øÆÕÍ¨ÖÕ¶Ë
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
//	if(direc==1) //´ÓĞ¡ºÅÖÕ¶Ëµ½´óºÅÖÕ¶Ë
//		{
//			content[3]
//		}
//	else if(direc==2) //´Ó´óºÅÖÕ¶Ëµ½Ğ¡ºÅÖÕ¶Ë
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
	a = read(0x14) & 0x8C;	//¶Á·¢ËÍ±êÖ¾
			if(a != 0)
			{
				if(a == 0x80)
				{
					write(0x0D,0x80);
					write(0x0D,0x00);	//Çåprepare_tx_flag±êÖ¾
				Launch_Announcement();//³¬ÉùĞÅºÅ·¢ÉäÍ¨¸æ£¬·¢Á½´Î£¬Ò»¸öÏòÇ°¶Ë·¢£¬Ò»¸öÏòºó¶Ë·¢
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
						a = read(0x15) & 0x88;//8;	//ÊÕµ½µÄÆäËûÖÕ¶ËÊä³ö·½²¨ÉÁµÆ
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
			a = read(0x0D) & 0x80;	//Õı³£½ÓÊÕ1
			if(a == 0x80)
			{
				a=read(0x25);
											#if	defined (USE_MC_DEBUG) && defined (PRINTF_DEBUG)		
													printf(" receive one ,a = %2x\r\n",a);
											#endif
				if((a&0xf0)== 0)//¸ßËÄÎ»Îª0
					{DG_StateOne=DG_StateOne & 0x0f;}//¸ß4Î»¸³Öµ0
				if((a&0x0f)== 0)//µÍËÄÎ»Îª0
					{DG_StateOne=DG_StateOne & 0xf0;}//µÍ4Î»¸³Öµ0
					
					
				if((a&0xf0)== 8)//¸ßËÄÎ»Îª0
					{
					 WaveFromFarAway=1;
											#if	defined (USE_MC_DEBUG) && defined (PRINTF_DEBUG)		
													printf(" receive wave from far away ,up,right\r\n");
											#endif
					}//
				if((a&0x0f)== 8)//µÍËÄÎ»Îª0
					{
						WaveFromFarAway=1;
											#if	defined (USE_MC_DEBUG) && defined (PRINTF_DEBUG)		
													printf(" receive wave from far away ,up,left\r\n");
											#endif
					}//
				
				write(0x0E,0x09);		//Ä¿µÄ£º´¥·¢³¬Ê±¼ÆÊı£¬Éè¼ÆÔÚÊÕµ½ÓĞĞ§Êı¾İºóÖØĞÂ¿ªÊ¼³¬Ê±¼ÆÊı
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
			
			a = read(0x0E) & 0x80;	//Õı³£½ÓÊÕ2
			if(a == 0x80)
			{
				a=read(0x26);
											#if	defined (USE_MC_DEBUG) && defined (PRINTF_DEBUG)		
													printf(" receive two ,a = %2x\r\n",a);
											#endif
				if((a&0xf0)== 0)//¸ßËÄÎ»Îª0
					{DG_StateTwo=DG_StateTwo & 0x0f;}//¸ß4Î»¸³Öµ0
				if((a&0x0f)== 0)//µÍËÄÎ»Îª0
					{DG_StateTwo=DG_StateTwo & 0xf0;}//µÍ4Î»¸³Öµ0
					
				if((a&0xf0)== 8)//¸ßËÄÎ»Îª0
					{
						WaveFromFarAway=1;
											#if	defined (USE_MC_DEBUG) && defined (PRINTF_DEBUG)		
													printf(" receive wave from far away ,down,right\r\n");
											#endif
					}//
				if((a&0x0f)== 8)//µÍËÄÎ»Îª0
					{
						WaveFromFarAway=1;
											#if	defined (USE_MC_DEBUG) && defined (PRINTF_DEBUG)		
													printf(" receive wave from far away ,down,left\r\n");
											#endif
					}//
				
				write(0x10,0x09);		//Ä¿µÄ£º´¥·¢³¬Ê±¼ÆÊı£¬Éè¼ÆÔÚÊÕµ½ÓĞĞ§Êı¾İºóÖØĞÂ¿ªÊ¼³¬Ê±¼ÆÊı
				write(0x10,0x00);
			
				Led1_Blink_Times=4;
				Led1_counter=0;
				Led1_Freq_Param=2;
			
				a = 0;
			}
			
/**************************************************************************************/
			a = read(0x1E);//²éÑ¯·ùÖµĞÅÏ¢ÊÇ·ñ¿É¶Á
			if(a != 0)
			{	
				if(a == 0x10 ) //È¡ÉÏ¹ìÓÒ²à·ùÖµ
					{
						position_mul=1;
						read_dat(3,4,6);
					}
				else if (a == 0x80 ) //È¡ÉÏ¹ì×ó²à·ùÖµ
					{
						position_mul=0;
						read_dat(3,4,6);
					}
				else if (a == 0x01 ) //È¡ÏÂ¹ìÓÒ²à·ùÖµ
					{
						position_mul=1;
						read_dat(4,4,6);
					}
				else if (a == 0x08 ) //È¡ÏÂ¹ì×ó²à·ùÖµ
					{
						position_mul=0;
						read_dat(4,4,6);
					}
				
				write(0x14,0x88);//Çå¿É¶Á±êÖ¾
				write(0x14,0x00);//
				a = 0;
			}
			
			
/*****************************³¬Ê±ÅĞ¶Ï***************************************/
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
				
				if(a == 0x80)					//×óÉÏ³¬Ê±
				{				
											#if	defined (USE_MC_DEBUG) && defined (PRINTF_DEBUG)		
													printf(" left up time out \r\n");
											#endif
					DG_StateOne=(DG_StateOne & 0xf0) | 0x07;//µÍ4Î»¸³Öµ7
					write(0x11,0x80);
					
					/****************************±¾´Î³¬Ê±Ã»ÓĞ·ùÖµ£¬Òª°ÑÉÏÒ»´ÎµÃµ½µÄ·ùÖµĞÅÏ¢ÇåÁã****************************/
					for(i=0;i<16;i++)
						{
							SignalAmplitudeInfBuf[i]=0;
						}
					/**************************************************************************/
					Led4_Blink_Times=2;
					Led4_counter=0;
					Led4_Freq_Param=8;
				}
				else if(a == 0x08)		//×óÏÂ³¬Ê±
				{
											#if	defined (USE_MC_DEBUG) && defined (PRINTF_DEBUG)		
													printf(" left down time out \r\n");
											#endif
					DG_StateTwo=(DG_StateTwo & 0xf0) | 0x07;//µÍ4Î»¸³Öµ7
					write(0x11,0x80);
					
						/****************************±¾´Î³¬Ê±Ã»ÓĞ·ùÖµ£¬Òª°ÑÉÏÒ»´ÎµÃµ½µÄ·ùÖµĞÅÏ¢ÇåÁã****************************/
					for(i=0;i<16;i++)
						{
							SignalAmplitudeInfBuf[i+32]=0;
						}
					/**************************************************************************/
					Led4_Blink_Times=2;
					Led4_counter=0;
					Led4_Freq_Param=8;
				}
				else if(a == 0x10)		//ÓÒÉÏ³¬Ê±
				{
											#if	defined (USE_MC_DEBUG) && defined (PRINTF_DEBUG)		
													printf(" right up time out \r\n");
											#endif
					DG_StateOne=(DG_StateOne & 0x0f) | 0x70;//¸ß4Î»¸³Öµ7
					write(0x11,0x80);
					
						/****************************±¾´Î³¬Ê±Ã»ÓĞ·ùÖµ£¬Òª°ÑÉÏÒ»´ÎµÃµ½µÄ·ùÖµĞÅÏ¢ÇåÁã****************************/
					for(i=0;i<16;i++)
						{
							SignalAmplitudeInfBuf[i+16]=0;
						}
					/**************************************************************************/
					Led4_Blink_Times=2;
					Led4_counter=0;
					Led4_Freq_Param=8;
				}
				else if(a == 0x01)		//ÓÒÏÂ³¬Ê±
				{
											#if	defined (USE_MC_DEBUG) && defined (PRINTF_DEBUG)		
													printf(" right down time out \r\n");
											#endif
					DG_StateTwo=(DG_StateTwo & 0x0f) | 0x70;//¸ß4Î»¸³Öµ7
					write(0x11,0x80);
					
						/****************************±¾´Î³¬Ê±Ã»ÓĞ·ùÖµ£¬Òª°ÑÉÏÒ»´ÎµÃµ½µÄ·ùÖµĞÅÏ¢ÇåÁã****************************/
					for(i=0;i<16;i++)
						{
							SignalAmplitudeInfBuf[i+48]=0;
						}
					/**************************************************************************/
					Led4_Blink_Times=2;
					Led4_counter=0;
					Led4_Freq_Param=8;
				}
				
				
				else if(a == 0x11)		//ÓÒÏÂÓÒÉÏÍ¬Ê±³¬Ê±
				{
											#if	defined (USE_MC_DEBUG) && defined (PRINTF_DEBUG)		
													printf(" right up and down time out \r\n");
											#endif
					DG_StateOne=(DG_StateOne & 0x0f) | 0x70;//¸ß4Î»¸³Öµ7
					DG_StateTwo=(DG_StateTwo & 0x0f) | 0x70;//¸ß4Î»¸³Öµ7
					write(0x11,0x80);
					
						/****************************±¾´Î³¬Ê±Ã»ÓĞ·ùÖµ£¬Òª°ÑÉÏÒ»´ÎµÃµ½µÄ·ùÖµĞÅÏ¢ÇåÁã****************************/
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
				
				else if(a == 0x88)		//×óÏÂ×óÉÏÍ¬Ê±³¬Ê±
				{
											#if	defined (USE_MC_DEBUG) && defined (PRINTF_DEBUG)		
													printf(" left up and down time out \r\n");
											#endif
					DG_StateOne=(DG_StateOne & 0xf0) | 0x07;//µÍ4Î»¸³Öµ7
					DG_StateTwo=(DG_StateTwo & 0xf0) | 0x07;//µÍ4Î»¸³Öµ7
					write(0x11,0x80);
					
						/****************************±¾´Î³¬Ê±Ã»ÓĞ·ùÖµ£¬Òª°ÑÉÏÒ»´ÎµÃµ½µÄ·ùÖµĞÅÏ¢ÇåÁã****************************/
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
				
				
				else if(a == 0x99)		//ÉÏÏÂ×óÓÒÈ«¶¼³¬Ê±
				{
											#if	defined (USE_MC_DEBUG) && defined (PRINTF_DEBUG)		
													printf(" all time out \r\n");
											#endif
					DG_StateOne=0x77;
					DG_StateTwo=0x77;//µÍ4Î»¸³Öµ7
					write(0x11,0x80);
					
						/****************************±¾´Î³¬Ê±Ã»ÓĞ·ùÖµ£¬Òª°ÑÉÏÒ»´ÎµÃµ½µÄ·ùÖµĞÅÏ¢ÇåÁã****************************/
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
	if((databuf[0].datatype== comtype || databuf[0].datatype == 0x88 )&& databuf[0].data_used == 0 && databuf[0].data_direction !=comdirect)//µÈ´ıµÄÊı¾İ»ØÓ¦¿ÉÄÜÊÇÕı³£µÄwaitdattype£¬Ò²¿ÉÄÜÊÇÒì³£µÄ0x88
		{databuf[0].data_read =1; return 0;}
	else if((databuf[1].datatype== comtype  || databuf[1].datatype == 0x88 )&& databuf[1].data_used == 0 && databuf[0].data_direction !=comdirect)
		{databuf[1].data_read =1; return 1;}
	else if((databuf[2].datatype== comtype  || databuf[2].datatype == 0x88 )&& databuf[2].data_used == 0 && databuf[0].data_direction !=comdirect)
		{databuf[2].data_read =1; return 2;}
	else
			{return 3;}

}

void ImeAck_Time_Param_Init(uint16_t waitime)//µÈ´ıwaitime*10ºÁÃë
{
	GetImmediaAck_Flag=0;
	ImeAck_Direction=0;
	ImeAckOk_Flag=0;
	ImeAckType=0XFF;
	
	WaitDataTime=waitime;
	
	/*************ÕâÑù×öÏàµ±ÓÚÇå¿Õ½ÓÊÕ»º³åÇø*********************/
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
	
	if(WAITDATABUF[waitData_Trigerednum]->command_comfrom ==1 )//¸ÃÖ¸ÁîÓÉPC·¢³ö
		{
				SendBufOne[4]=TerminalBasicInf->LocalTerminalAddr;//oriaddr
				SendBufOne[5]=0xff;//desaddr
				if(WAITDATABUF[waitData_Trigerednum]->command_direction == 1)//ÕıÏò
					{		
						SendBufOne[7]=TerminalBasicInf->NextTerminalAddr;//³ö´íµÄÊÇºóÒ»¶Ë
					}
				else if(WAITDATABUF[waitData_Trigerednum]->command_direction == 2)//·´Ïò
					{				
						SendBufOne[7]=TerminalBasicInf->FrontTerminalAddr;//³ö´íµÄÊÇÇ°Ò»¶Ë
					}
		}
	else //¸ÃÖ¸ÁîÓÉÏàÁÚÖÕ¶Ë·¢³ö
		{
				SendBufOne[4]=TerminalBasicInf->LocalTerminalAddr;//oriaddr¸ÄÎª±¾¶ËµØÖ·
				if(WAITDATABUF[waitData_Trigerednum]->command_direction == 1)//ÕıÏò
					{		
						SendBufOne[5]=TerminalBasicInf->FrontTerminalAddr;
						SendBufOne[7]=TerminalBasicInf->NextTerminalAddr;//³ö´íµÄÊÇºóÒ»¶Ë
					}
				else if(WAITDATABUF[waitData_Trigerednum]->command_direction == 2)//·´Ïò
					{				
						SendBufOne[5]=TerminalBasicInf->NextTerminalAddr;
						SendBufOne[7]=TerminalBasicInf->FrontTerminalAddr;//³ö´íµÄÊÇÇ°Ò»¶Ë
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
	
	if((board_temperature&0xf000)==0xf000) //ÎÂ¶ÈÊÇ0ÏÂµÄ
		{
			simple_boardtemperature=255-simple_boardtemperature;
			simple_boardtemperature=simple_boardtemperature|0x80;//1000_0000
			TemperatureInfBuf[0]=simple_boardtemperature;
//			TemperatureInfBuf[2]=simple_boardtemperature;
		}
	else //ÎÂ¶ÈÊÇ0ÉÏµÄ
		{
			TemperatureInfBuf[0]=simple_boardtemperature;
//			TemperatureInfBuf[2]=simple_boardtemperature;
		}
	
		board_temperature=ReadTempurature_OUT();	
	
		simple_boardtemperature=(uint8_t)((board_temperature&0x0F00)>>4)+(uint8_t)((board_temperature&0x00F0)>>4);
	
	if((board_temperature&0xf000)==0xf000) //ÎÂ¶ÈÊÇ0ÏÂµÄ
		{
			simple_boardtemperature=255-simple_boardtemperature;
			simple_boardtemperature=simple_boardtemperature|0x80;//1000_0000
			TemperatureInfBuf[1]=simple_boardtemperature;
			TemperatureInfBuf[3]=simple_boardtemperature;
		}
	else //ÎÂ¶ÈÊÇ0ÉÏµÄ
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
	/*******************×°ÔØÊ±¼ä***********************/
	for(i=0;i<6;i++)
		{
		SendBufOne[i+8]=RccInfBuf[i];
		}
	/*******************×°ÔØÍ¨¶ÏĞÅÏ¢***********************/
		SendBufOne[14]=DGStateInfBuf[0];
		SendBufOne[53]=DGStateInfBuf[1];
	/*******************×°ÔØÓ¦Á¦ĞÅÏ¢***********************/
		SendBufOne[15]=StressInfBuf[0];
		SendBufOne[16]=StressInfBuf[1];
		SendBufOne[54]=StressInfBuf[2];
		SendBufOne[55]=StressInfBuf[3];
		/*******************×°ÔØÎÂ¶ÈĞÅÏ¢***********************/
		SendBufOne[17]=TemperatureInfBuf[0];
		SendBufOne[18]=TemperatureInfBuf[1];
		SendBufOne[56]=TemperatureInfBuf[2];
		SendBufOne[57]=TemperatureInfBuf[3];
		/*******************×°ÔØ±¾¶ËĞÅºÅ·ùÖµ***********************/
		SendBufOne[19]=LocalSignalAmplitudeInfBuf[0];
		SendBufOne[20]=LocalSignalAmplitudeInfBuf[1];
		SendBufOne[58]=LocalSignalAmplitudeInfBuf[2];
		SendBufOne[59]=LocalSignalAmplitudeInfBuf[3];
		/*******************×°ÔØÔ¶¶ËĞÅºÅ·ùÖµĞÅÏ¢***********************/
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
			if(waitdata_registnum==3)//3ºÅ½á¹¹Ìå¸Õ¸Õ±»ÓÃ¹ıÁË£¬ĞÂµÄÊı¾İ×¢²áµ½3ºÅ½á¹¹ÌåÖ®ºóµÄÒ»ºÅ½á¹¹ÌåÀï
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
			else if(waitdata_registnum==1)//1ºÅ½á¹¹Ìå¸Õ¸Õ±»ÓÃ¹ıÁË£¬ĞÂµÄÊı¾İ×¢²áµ½1ºÅ½á¹¹ÌåÖ®ºóµÄ¶şºÅ½á¹¹ÌåÀï
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
				else if(waitdata_registnum==2)//2ºÅ½á¹¹Ìå¸Õ¸Õ±»ÓÃ¹ıÁË£¬ĞÂµÄÊı¾İ×¢²áµ½2ºÅ½á¹¹ÌåÖ®ºóµÄÈıºÅ½á¹¹ÌåÀï
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
						if(WAITDATABUF[0]->WaitDataFlag == 2)//ĞèÒªµÈ´ıÁ¢¼´ÏìÓ¦
							{
									ImeAckOk_Flag=WaitIme_Ack(GetImmediaAck_Flag,ImeAck_Direction,ImeAckType,
																							WAITDATABUF[0]->command_direction,
																							WAITDATABUF[0]->command_type);
								/************************»¹´¦ÔÚ½ÓÊÕÊ±ÏŞÄÚ********************************/
									if(WAITDATABUF[0]->waitdata_timelimit > waitdata_time[0] ) 
									{
										/*********************Èç¹ûµÈµ½ÁËÁ¢¼´ÏìÓ¦¾Í°ÑWaitDataFlagÖÃ1£¬¼ÌĞøµÈ´ıÊı¾İ*********************************/
										if(ImeAckOk_Flag == 1)  
											{
													WAITDATABUF[0]->WaitDataFlag=1;
													WAITDATABUF[0]->waitdata_timelimit=300;//300´ú±íµÈ´ıÊı¾İµÄÊ±¼ä¿ÉÒÔÊÇºÜ³¤£¬´ó¸Å15Ãë
											}
											/***********µÈ´ıÊ±ÏŞÄÚÃ»ÊÕµ½ÏëÒªµÄÊı¾İ£¬ÄÇ¾Í¼ÌĞøÈ¥×öÆäËûÊÂÇé*******************************************/
										else
											{tastate=TsakSW_Analy;}
									}
									/************************ÒÑ¾­²»ÔÚ½ÓÊÕÊ±ÏŞÄÚÁË,Çå³ıµÈ´ıÊı¾İµÄ±êÖ¾£¬Í¬Ê±Ìø×ª±¸ÓÃĞĞ¶¯²½Öè********************************/
									else
									{
										WAITDATABUF[0]->WaitDataFlag=0;
										Com_Param=WAITDATABUF[0]->comparam_copy;
										Aim_Param=WAITDATABUF[0]->aimparam_copy;
										step_num=WAITDATABUF[0]->step_num_copy+4;
										
										/*****************ÒÑ¾­²úÉúµÄĞĞ¶¯²½Öè²»ÄÜ±»2£¬3×¢²áÇø²úÉúµÄĞĞ¶¯²½ÖèÆÆ»µµô*******************/
										buftwo_disable=1;
										bufthree_disable=1;
										/************************************************/
										GoToNextStep();
										
													#if	defined (USE_MC_DEBUG) && defined (PRINTF_DEBUG)		
																	printf("\r\n no ime ack, WAITDATABUF[0] cancel ,the waitdata type is %2x\r\n",WAITDATABUF[0]->command_type);
													#endif
									}
							}
						else if(WAITDATABUF[0]->WaitDataFlag == 1 && WAITDATABUF[0]->noneed_dataresponse == 1) //²»ĞèÒªµÈ´ıÊı¾İ
							{
							/*****************Çå¿ÕµÈ´ıÊı¾İµÄ±êÖ¾£¬È»ºó¼ÌĞøÈ¥×öÆäËûÊÂÇé***********************/
								WAITDATABUF[0]->WaitDataFlag=0;
								tastate=TsakSW_Analy;
													#if	defined (USE_MC_DEBUG) && defined (PRINTF_DEBUG)		
																	printf("\r\n  WAITDATABUF[0] finished ,the waitdata type is %2x\r\n",WAITDATABUF[0]->command_type);
													#endif
							}
						else if(WAITDATABUF[0]->WaitDataFlag == 1 && WAITDATABUF[0]->noneed_dataresponse == 2) //¿ÉÄÜĞèÒªµÈ´ıÊ§Áª±¨¸æ
							{
									DataAckOk_Flag=WaitData_Ack(DATABUF[0],WAITDATABUF[0]->command_direction,0x88);
										/************************»¹´¦ÔÚ½ÓÊÕÊ±ÏŞÄÚ********************************/
									if(WAITDATABUF[0]->waitdata_timelimit > waitdata_time[0]) 
										{
											/*********************Èç¹ûµÈµ½ÁËÏëÒªµÄÊı¾İ¾ÍÓÃÊı¾İ´¥·¢ĞÂµÄĞĞ¶¯²½Öè£¬Í¬Ê±Çå³ıµÈ´ı±êÖ¾*********************************/
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
												/***********µÈ´ıÊ±ÏŞÄÚÃ»ÊÕµ½ÏëÒªµÄÊı¾İ£¬ÄÇ¾Í¼ÌĞøÈ¥×öÆäËûÊÂÇé*******************************************/
											else
												{tastate=TsakSW_Analy;}
										}
									/************************ÒÑ¾­²»ÔÚ½ÓÊÕÊ±ÏŞÄÚÁË,Çå³ıµÈ´ıÊı¾İµÄ±êÖ¾£¬È»ºó¼ÌĞø×öÆäËûÊÂÇé********************************/
									else
										{
											WAITDATABUF[0]->WaitDataFlag=0;
											tastate=TsakSW_Analy;
											
											#if	defined (USE_MC_DEBUG) && defined (PRINTF_DEBUG)		
																	printf("\r\n  WAITDATABUF[0] cancel ,the waitdata type is %2x\r\n",0x88);
													#endif
										}
							
							}
						else if(WAITDATABUF[0]->WaitDataFlag == 1  )//ĞèÒªµÈ´ıÊı¾İ
							{
									DataAckOk_Flag=WaitData_Ack(DATABUF[0],WAITDATABUF[0]->command_direction,WAITDATABUF[0]->command_type);
										/************************»¹´¦ÔÚ½ÓÊÕÊ±ÏŞÄÚ********************************/
									if(WAITDATABUF[0]->waitdata_timelimit > waitdata_time[0]) 
										{
											/*********************Èç¹ûµÈµ½ÁËÏëÒªµÄÊı¾İ¾ÍÓÃÊı¾İ´¥·¢ĞÂµÄĞĞ¶¯²½Öè£¬Í¬Ê±Çå³ıµÈ´ı±êÖ¾*********************************/
											if(DataAckOk_Flag < 3 )  
												{
														WAITDATABUF[0]->WaitDataFlag=0;
														
														DataPeriod=1;
														DatabufNum=DataAckOk_Flag;
													
														/*****************ÒÑ¾­²úÉúµÄĞĞ¶¯²½Öè²»ÄÜ±»2£¬3×¢²áÇø²úÉúµÄĞĞ¶¯²½ÖèÆÆ»µµô*******************/
														buftwo_disable=1;
														bufthree_disable=1;
														/************************************************/
							
														tastate=TA_OOM_Init;
													#if	defined (USE_MC_DEBUG) && defined (PRINTF_DEBUG)		
																	printf("\r\n  WAITDATABUF[0] finished ,the waitdata type is %2x\r\n",WAITDATABUF[0]->command_type);
													#endif
												}
												/***********µÈ´ıÊ±ÏŞÄÚÃ»ÊÕµ½ÏëÒªµÄÊı¾İ£¬ÄÇ¾Í¼ÌĞøÈ¥×öÆäËûÊÂÇé*******************************************/
											else
												{tastate=TsakSW_Analy;}
										}
									/************************ÒÑ¾­²»ÔÚ½ÓÊÕÊ±ÏŞÄÚÁË,Çå³ıµÈ´ıÊı¾İµÄ±êÖ¾£¬Í¬Ê±Ìø×ª±¸ÓÃĞĞ¶¯²½Öè********************************/
									else
										{
											WAITDATABUF[0]->WaitDataFlag=0;
											Com_Param=WAITDATABUF[0]->comparam_copy;
											Aim_Param=WAITDATABUF[0]->aimparam_copy;
											step_num=WAITDATABUF[0]->step_num_copy+4;
											
											/*****************ÒÑ¾­²úÉúµÄĞĞ¶¯²½Öè²»ÄÜ±»2£¬3×¢²áÇø²úÉúµÄĞĞ¶¯²½ÖèÆÆ»µµô*******************/
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
						if(WAITDATABUF[1]->WaitDataFlag == 2)//ĞèÒªµÈ´ıÁ¢¼´ÏìÓ¦
							{
									ImeAckOk_Flag=WaitIme_Ack(GetImmediaAck_Flag,ImeAck_Direction,ImeAckType,
																							WAITDATABUF[1]->command_direction,
																							WAITDATABUF[1]->command_type);
								/************************»¹´¦ÔÚ½ÓÊÕÊ±ÏŞÄÚ********************************/
									if(WAITDATABUF[1]->waitdata_timelimit > waitdata_time[1] ) 
									{
										/*********************Èç¹ûµÈµ½ÁËÁ¢¼´ÏìÓ¦¾Í°ÑWaitDataFlagÖÃ1£¬¼ÌĞøµÈ´ıÊı¾İ*********************************/
										if(ImeAckOk_Flag == 1)  
											{
													WAITDATABUF[1]->WaitDataFlag=1;
													WAITDATABUF[1]->waitdata_timelimit=300;//300´ú±íµÈ´ıÊı¾İµÄÊ±¼ä¿ÉÒÔÊÇºÜ³¤£¬´ó¸Å15Ãë
											}
											/***********µÈ´ıÊ±ÏŞÄÚÃ»ÊÕµ½ÏëÒªµÄÊı¾İ£¬ÄÇ¾Í¼ÌĞøÈ¥×öÆäËûÊÂÇé*******************************************/
										else
											{tastate=TsakSW_Analy;}
									}
									/************************ÒÑ¾­²»ÔÚ½ÓÊÕÊ±ÏŞÄÚÁË,Çå³ıµÈ´ıÊı¾İµÄ±êÖ¾£¬Í¬Ê±Ìø×ª±¸ÓÃĞĞ¶¯²½Öè********************************/
									else
									{
										WAITDATABUF[1]->WaitDataFlag=0;
										Com_Param=WAITDATABUF[1]->comparam_copy;
										Aim_Param=WAITDATABUF[1]->aimparam_copy;
										step_num=WAITDATABUF[1]->step_num_copy+4;
										
														/*****************ÒÑ¾­²úÉúµÄĞĞ¶¯²½Öè²»ÄÜ±»3×¢²áÇø²úÉúµÄĞĞ¶¯²½ÖèÆÆ»µµô*******************/
														bufthree_disable=1;
														/************************************************/
										GoToNextStep();
										#if	defined (USE_MC_DEBUG) && defined (PRINTF_DEBUG)		
																	printf("\r\n no ime ack , WAITDATABUF[1] cancel ,the waitdata type is %2x\r\n",WAITDATABUF[1]->command_type);
													#endif
									}
							}
						else if(WAITDATABUF[1]->WaitDataFlag == 1 && WAITDATABUF[1]->noneed_dataresponse == 1) //²»ĞèÒªµÈ´ıÊı¾İ
							{
							/*****************Çå¿ÕµÈ´ıÊı¾İµÄ±êÖ¾£¬È»ºó¼ÌĞøÈ¥×öÆäËûÊÂÇé***********************/
								WAITDATABUF[1]->WaitDataFlag=0;
								tastate=TsakSW_Analy;
								#if	defined (USE_MC_DEBUG) && defined (PRINTF_DEBUG)		
																	printf("\r\n  WAITDATABUF[1] finished ,the waitdata type is %2x\r\n",WAITDATABUF[1]->command_type);
													#endif
							}
						else if(WAITDATABUF[1]->WaitDataFlag == 1 && WAITDATABUF[1]->noneed_dataresponse == 2) //¿ÉÄÜĞèÒªµÈ´ıÊ§Áª±¨¸æ
							{
									DataAckOk_Flag=WaitData_Ack(DATABUF[0],WAITDATABUF[1]->command_direction,0x88);
										/************************»¹´¦ÔÚ½ÓÊÕÊ±ÏŞÄÚ********************************/
									if(WAITDATABUF[1]->waitdata_timelimit > waitdata_time[1]) 
										{
											/*********************Èç¹ûµÈµ½ÁËÏëÒªµÄÊı¾İ¾ÍÓÃÊı¾İ´¥·¢ĞÂµÄĞĞ¶¯²½Öè£¬Í¬Ê±Çå³ıµÈ´ı±êÖ¾*********************************/
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
												/***********µÈ´ıÊ±ÏŞÄÚÃ»ÊÕµ½ÏëÒªµÄÊı¾İ£¬ÄÇ¾Í¼ÌĞøÈ¥×öÆäËûÊÂÇé*******************************************/
											else
												{tastate=TsakSW_Analy;}
										}
									/************************ÒÑ¾­²»ÔÚ½ÓÊÕÊ±ÏŞÄÚÁË,Çå³ıµÈ´ıÊı¾İµÄ±êÖ¾£¬È»ºó¼ÌĞø×öÆäËûÊÂÇé********************************/
									else
										{
											WAITDATABUF[1]->WaitDataFlag=0;
											tastate=TsakSW_Analy;
											
											#if	defined (USE_MC_DEBUG) && defined (PRINTF_DEBUG)		
																	printf("\r\n  WAITDATABUF[1] cancel ,the waitdata type is %2x\r\n",0x88);
													#endif
										}
							
							}
						else if(WAITDATABUF[1]->WaitDataFlag == 1)//ĞèÒªµÈ´ıÊı¾İ
							{
									DataAckOk_Flag=WaitData_Ack(DATABUF[0],WAITDATABUF[1]->command_direction,WAITDATABUF[1]->command_type);
										/************************»¹´¦ÔÚ½ÓÊÕÊ±ÏŞÄÚ********************************/
									if(WAITDATABUF[1]->waitdata_timelimit > waitdata_time[1] ) 
										{
											/*********************Èç¹ûµÈµ½ÁËÏëÒªµÄÊı¾İ¾ÍÓÃÊı¾İ´¥·¢ĞÂµÄĞĞ¶¯²½Öè£¬Í¬Ê±Çå³ıµÈ´ı±êÖ¾*********************************/
											if(DataAckOk_Flag < 3 )  
												{
														WAITDATABUF[1]->WaitDataFlag=0;
														
														DataPeriod=1;
														DatabufNum=DataAckOk_Flag;
													
													/*****************ÒÑ¾­²úÉúµÄĞĞ¶¯²½Öè²»ÄÜ±»3×¢²áÇø²úÉúµÄĞĞ¶¯²½ÖèÆÆ»µµô*******************/
														bufthree_disable=1;
														/************************************************/
													
														tastate=TA_OOM_Init;
													#if	defined (USE_MC_DEBUG) && defined (PRINTF_DEBUG)		
																	printf("\r\n  WAITDATABUF[1] finished ,the waitdata type is %2x\r\n",WAITDATABUF[1]->command_type);
													#endif
												}
												/***********µÈ´ıÊ±ÏŞÄÚÃ»ÊÕµ½ÏëÒªµÄÊı¾İ£¬ÄÇ¾Í¼ÌĞøÈ¥×öÆäËûÊÂÇé*******************************************/
											else
												{tastate=TsakSW_Analy;}
										}
									/************************ÒÑ¾­²»ÔÚ½ÓÊÕÊ±ÏŞÄÚÁË,Çå³ıµÈ´ıÊı¾İµÄ±êÖ¾£¬Í¬Ê±Ìø×ª±¸ÓÃĞĞ¶¯²½Öè********************************/
									else
										{
											WAITDATABUF[1]->WaitDataFlag=0;
											Com_Param=WAITDATABUF[1]->comparam_copy;
											Aim_Param=WAITDATABUF[1]->aimparam_copy;
											step_num=WAITDATABUF[1]->step_num_copy+4;
											
											/*****************ÒÑ¾­²úÉúµÄĞĞ¶¯²½Öè²»ÄÜ±»3×¢²áÇø²úÉúµÄĞĞ¶¯²½ÖèÆÆ»µµô*******************/
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
						if(WAITDATABUF[2]->WaitDataFlag == 2)//ĞèÒªµÈ´ıÁ¢¼´ÏìÓ¦
							{
									ImeAckOk_Flag=WaitIme_Ack(GetImmediaAck_Flag,ImeAck_Direction,ImeAckType,
																							WAITDATABUF[2]->command_direction,
																							WAITDATABUF[2]->command_type);
								/************************»¹´¦ÔÚ½ÓÊÕÊ±ÏŞÄÚ********************************/
									if(WAITDATABUF[2]->waitdata_timelimit > waitdata_time[2] ) 
									{
										/*********************Èç¹ûµÈµ½ÁËÁ¢¼´ÏìÓ¦¾Í°ÑWaitDataFlagÖÃ1£¬¼ÌĞøµÈ´ıÊı¾İ*********************************/
										if(ImeAckOk_Flag == 1)  
											{
													WAITDATABUF[2]->WaitDataFlag=1;
													WAITDATABUF[2]->waitdata_timelimit=300;//300´ú±íµÈ´ıÊı¾İµÄÊ±¼ä¿ÉÒÔÊÇºÜ³¤£¬´ó¸Å15Ãë
											}
											/***********µÈ´ıÊ±ÏŞÄÚÃ»ÊÕµ½ÏëÒªµÄÊı¾İ£¬ÄÇ¾Í¼ÌĞøÈ¥×öÆäËûÊÂÇé*******************************************/
										else
											{tastate=TsakSW_Analy;}
									}
									/************************ÒÑ¾­²»ÔÚ½ÓÊÕÊ±ÏŞÄÚÁË,Çå³ıµÈ´ıÊı¾İµÄ±êÖ¾£¬Í¬Ê±Ìø×ª±¸ÓÃĞĞ¶¯²½Öè********************************/
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
						else if(WAITDATABUF[2]->WaitDataFlag == 1 && WAITDATABUF[2]->noneed_dataresponse == 1) //²»ĞèÒªµÈ´ıÊı¾İ
							{
							/*****************Çå¿ÕµÈ´ıÊı¾İµÄ±êÖ¾£¬È»ºó¼ÌĞøÈ¥×öÆäËûÊÂÇé***********************/
								WAITDATABUF[2]->WaitDataFlag=0;
								tastate=TsakSW_Analy;
													#if	defined (USE_MC_DEBUG) && defined (PRINTF_DEBUG)		
																	printf("\r\n  WAITDATABUF[2] finished ,the waitdata type is %2x\r\n",WAITDATABUF[2]->command_type);
													#endif
							}
						else if(WAITDATABUF[2]->WaitDataFlag == 1 && WAITDATABUF[2]->noneed_dataresponse == 2) //¿ÉÄÜĞèÒªµÈ´ıÊ§Áª±¨¸æ
							{
									DataAckOk_Flag=WaitData_Ack(DATABUF[0],WAITDATABUF[2]->command_direction,0x88);
										/************************»¹´¦ÔÚ½ÓÊÕÊ±ÏŞÄÚ********************************/
									if(WAITDATABUF[2]->waitdata_timelimit > waitdata_time[2]) 
										{
											/*********************Èç¹ûµÈµ½ÁËÏëÒªµÄÊı¾İ¾ÍÓÃÊı¾İ´¥·¢ĞÂµÄĞĞ¶¯²½Öè£¬Í¬Ê±Çå³ıµÈ´ı±êÖ¾*********************************/
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
												/***********µÈ´ıÊ±ÏŞÄÚÃ»ÊÕµ½ÏëÒªµÄÊı¾İ£¬ÄÇ¾Í¼ÌĞøÈ¥×öÆäËûÊÂÇé*******************************************/
											else
												{tastate=TsakSW_Analy;}
										}
									/************************ÒÑ¾­²»ÔÚ½ÓÊÕÊ±ÏŞÄÚÁË,Çå³ıµÈ´ıÊı¾İµÄ±êÖ¾£¬È»ºó¼ÌĞø×öÆäËûÊÂÇé********************************/
									else
										{
											WAITDATABUF[2]->WaitDataFlag=0;
											tastate=TsakSW_Analy;
											
											#if	defined (USE_MC_DEBUG) && defined (PRINTF_DEBUG)		
																	printf("\r\n  WAITDATABUF[2] cancel ,the waitdata type is %2x\r\n",0x88);
													#endif
										}
							
							}
						else if(WAITDATABUF[2]->WaitDataFlag == 1)//ĞèÒªµÈ´ıÊı¾İ
							{
									DataAckOk_Flag=WaitData_Ack(DATABUF[0],WAITDATABUF[2]->command_direction,WAITDATABUF[2]->command_type);
										/************************»¹´¦ÔÚ½ÓÊÕÊ±ÏŞÄÚ********************************/
									if(WAITDATABUF[2]->waitdata_timelimit > waitdata_time[2]) 
										{
											/*********************Èç¹ûµÈµ½ÁËÏëÒªµÄÊı¾İ¾ÍÓÃÊı¾İ´¥·¢ĞÂµÄĞĞ¶¯²½Öè£¬Í¬Ê±Çå³ıµÈ´ı±êÖ¾*********************************/
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
												/***********µÈ´ıÊ±ÏŞÄÚÃ»ÊÕµ½ÏëÒªµÄÊı¾İ£¬ÄÇ¾Í¼ÌĞøÈ¥×öÆäËûÊÂÇé*******************************************/
											else
												{tastate=TsakSW_Analy;}
										}
									/************************ÒÑ¾­²»ÔÚ½ÓÊÕÊ±ÏŞÄÚÁË,Çå³ıµÈ´ıÊı¾İµÄ±êÖ¾£¬Í¬Ê±Ìø×ª±¸ÓÃĞĞ¶¯²½Öè********************************/
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
ÔÚ·ÖÎöµÈ´ıÊı¾İµÄ¹ı³ÌÖĞĞèÒªµÈ´ıÁ¢¼´ÏìÓ¦µÄÊ±ºòµ÷ÓÃ¸Ãº¯Êı

*********************************************************************/
void ImeAck_Handle(uint8_t wdr_bufnum)
{
		ImeAckOk_Flag=WaitIme_Ack(GetImmediaAck_Flag,ImeAck_Direction,ImeAckType,
																							WAITDATABUF[wdr_bufnum]->command_direction,
																							WAITDATABUF[wdr_bufnum]->command_type);
			/************************»¹´¦ÔÚ½ÓÊÕÊ±ÏŞÄÚ********************************/
				if(WAITDATABUF[wdr_bufnum]->waitdata_timelimit > waitdata_time[wdr_bufnum] ) 
				{
					/*********************Èç¹ûµÈµ½ÁËÁ¢¼´ÏìÓ¦¾Í°ÑWaitDataFlagÖÃ1£¬¼ÌĞøµÈ´ıÊı¾İ*********************************/
					if(ImeAckOk_Flag == 1)  
						{
								WAITDATABUF[wdr_bufnum]->WaitDataFlag=1;
								WAITDATABUF[wdr_bufnum]->waitdata_timelimit=300;//300´ú±íµÈ´ıÊı¾İµÄÊ±¼ä¿ÉÒÔÊÇºÜ³¤£¬´ó¸Å15Ãë
						}
						/***********µÈ´ıÊ±ÏŞÄÚÃ»ÊÕµ½ÏëÒªµÄÊı¾İ£¬ÄÇ¾Í¼ÌĞøÈ¥×öÆäËûÊÂÇé*******************************************/
					else
						{tastate=TsakSW_Analy;}
				}
				/************************ÒÑ¾­²»ÔÚ½ÓÊÕÊ±ÏŞÄÚÁË,Çå³ıµÈ´ıÊı¾İµÄ±êÖ¾£¬Í¬Ê±Ìø×ª±¸ÓÃĞĞ¶¯²½Öè********************************/
				else
				{
					WAITDATABUF[wdr_bufnum]->WaitDataFlag=0;
					Com_Param=WAITDATABUF[wdr_bufnum]->comparam_copy;
					Aim_Param=WAITDATABUF[wdr_bufnum]->aimparam_copy;
					step_num=WAITDATABUF[wdr_bufnum]->step_num_copy+4;
					
					/*****************ÒÑ¾­²úÉúµÄĞĞ¶¯²½Öè²»ÄÜ±»ÆäËû×¢²áÇø²úÉúµÄĞĞ¶¯²½ÖèÆÆ»µµô*******************/
						WaitDataAnalysisOver_Flag=1;
					/************************************************/
					GoToNextStep();
					
								#if	defined (USE_MC_DEBUG) && defined (PRINTF_DEBUG)		
												printf("\r\n no ime ack, WAITDATABUF[%d] cancel ,the waitdata type is %2x\r\n",wdr_bufnum,WAITDATABUF[wdr_bufnum]->command_type);
								#endif
				}
}
/*****************Çå¿ÕµÈ´ıÊı¾İµÄ±êÖ¾£¬È»ºó¼ÌĞøÈ¥×öÆäËûÊÂÇé***********************/
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
			/************************»¹´¦ÔÚ½ÓÊÕÊ±ÏŞÄÚ********************************/
		if(WAITDATABUF[wdr_bufnum]->waitdata_timelimit > waitdata_time[wdr_bufnum]) 
			{
				/*********************Èç¹ûµÈµ½ÁËÏëÒªµÄÊı¾İ¾ÍÓÃÊı¾İ´¥·¢ĞÂµÄĞĞ¶¯²½Öè£¬Í¬Ê±Çå³ıµÈ´ı±êÖ¾*********************************/
				if(DataAckOk_Flag < 3 && DATABUF[DataAckOk_Flag]->datatype == 0x88)  
					{
							WAITDATABUF[wdr_bufnum]->WaitDataFlag=0;
							
							DataPeriod=1;
							DatabufNum=DataAckOk_Flag;
						
							tastate=TA_OOM_Init;
							/*****************ÒÑ¾­²úÉúµÄĞĞ¶¯²½Öè²»ÄÜ±»ÆäËû×¢²áÇø²úÉúµÄĞĞ¶¯²½ÖèÆÆ»µµô*******************/
							WaitDataAnalysisOver_Flag=1;
							/************************************************/
							#if	defined (USE_MC_DEBUG) && defined (PRINTF_DEBUG)		
										printf("\r\n  WAITDATABUF[%d] finished ,the waitdata type is %2x\r\n",wdr_bufnum,0x88);
						#endif
					}
					/***********µÈ´ıÊ±ÏŞÄÚÃ»ÊÕµ½ÏëÒªµÄÊı¾İ£¬ÄÇ¾Í¼ÌĞøÈ¥×öÆäËûÊÂÇé*******************************************/
				else
					{tastate=TsakSW_Analy;}
			}
		/************************ÒÑ¾­²»ÔÚ½ÓÊÕÊ±ÏŞÄÚÁË,Çå³ıµÈ´ıÊı¾İµÄ±êÖ¾£¬È»ºó¼ÌĞø×öÆäËûÊÂÇé********************************/
		else
			{
				WAITDATABUF[wdr_bufnum]->WaitDataFlag=0;
				tastate=TsakSW_Analy;
				
				#if	defined (USE_MC_DEBUG) && defined (PRINTF_DEBUG)		
										printf("\r\n  WAITDATABUF[%d] cancel ,the waitdata type is %2x\r\n",wdr_bufnum,0x88);
						#endif
			}
}

void WaitOneData_Handle(uint8_t wdr_bufnum,uint8_t accept_lose_report)//accept_lose_reportÎª0Ê±´ú±í²»½ÓÊÜÊ§Áª±¨¸æ
{
		DataAckOk_Flag=WaitData_Ack(DATABUF[0],WAITDATABUF[wdr_bufnum]->command_direction,WAITDATABUF[wdr_bufnum]->command_type);
			/************************»¹´¦ÔÚ½ÓÊÕÊ±ÏŞÄÚ********************************/
		if(WAITDATABUF[wdr_bufnum]->waitdata_timelimit > waitdata_time[wdr_bufnum]) 
			{
				/*********************Èç¹ûµÈµ½ÁËÏëÒªµÄÊı¾İ¾ÍÓÃÊı¾İ´¥·¢ĞÂµÄĞĞ¶¯²½Öè£¬Í¬Ê±Çå³ıµÈ´ı±êÖ¾*********************************/
				if(DataAckOk_Flag < 3 && (DATABUF[DataAckOk_Flag]->datatype != 0x88 || accept_lose_report == 1))  
					{
							WAITDATABUF[wdr_bufnum]->WaitDataFlag=0;
							
							DataPeriod=1;
							DatabufNum=DataAckOk_Flag;
						
							/*****************ÒÑ¾­²úÉúµÄĞĞ¶¯²½Öè²»ÄÜ±»ÆäËû×¢²áÇø²úÉúµÄĞĞ¶¯²½ÖèÆÆ»µµô*******************/
						WaitDataAnalysisOver_Flag=1;
					/************************************************/

							tastate=TA_OOM_Init;
						#if	defined (USE_MC_DEBUG) && defined (PRINTF_DEBUG)		
										printf("\r\n  WAITDATABUF[%d] finished ,the waitdata type is %2x\r\n",wdr_bufnum,WAITDATABUF[wdr_bufnum]->command_type);
						#endif
					}
					/***********µÈ´ıÊ±ÏŞÄÚÃ»ÊÕµ½ÏëÒªµÄÊı¾İ£¬ÄÇ¾Í¼ÌĞøÈ¥×öÆäËûÊÂÇé*******************************************/
				else
					{tastate=TsakSW_Analy;}
			}
		/************************ÒÑ¾­²»ÔÚ½ÓÊÕÊ±ÏŞÄÚÁË,Çå³ıµÈ´ıÊı¾İµÄ±êÖ¾£¬Í¬Ê±Ìø×ª±¸ÓÃĞĞ¶¯²½Öè********************************/
		else
			{
				WAITDATABUF[wdr_bufnum]->WaitDataFlag=0;
				Com_Param=WAITDATABUF[wdr_bufnum]->comparam_copy;
				Aim_Param=WAITDATABUF[wdr_bufnum]->aimparam_copy;
				step_num=WAITDATABUF[wdr_bufnum]->step_num_copy+4;
				
				/*****************ÒÑ¾­²úÉúµÄĞĞ¶¯²½Öè²»ÄÜ±»ÆäËû×¢²áÇø²úÉúµÄĞĞ¶¯²½ÖèÆÆ»µµô*******************/
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
			/************************»¹´¦ÔÚ½ÓÊÕÊ±ÏŞÄÚ********************************/
		if(WAITDATABUF[wdr_bufnum]->waitdata_timelimit > waitdata_time[wdr_bufnum]) 
			{
				/*********************Èç¹ûµÈµ½ÁËÏëÒªµÄÊı¾İ¾ÍÓÃÊı¾İ´¥·¢ĞÂµÄĞĞ¶¯²½Öè£¬*********************************/
				if(DataAckOk_Flag < 3 && (DATABUF[DataAckOk_Flag]->datatype != 0x88 || accept_lose_report == 1))  
					{
							(WAITDATABUF[wdr_bufnum]->WaitDataResponse_Quantity)--;
							waitdata_time[wdr_bufnum]=0;//µÈ´ıÊ±¼äÀÛ¼Ó¼ÆÊıÆ÷Çå0£¬ÒÔ±ãµÈ´ıÏÂÒ»´ÎÊı¾İ
							DataPeriod=1;
							DatabufNum=DataAckOk_Flag;
						
							if(DATABUF[DataAckOk_Flag]->datatype == 0x88) 
								{
									WAITDATABUF[wdr_bufnum]->WaitDataFlag=0;
											#if	defined (USE_MC_DEBUG) && defined (PRINTF_DEBUG)		
												printf("\r\n  WAITDATABUF[%d] cancel ,the waitdata type is %2x\r\n",wdr_bufnum,WAITDATABUF[wdr_bufnum]->command_type);
											#endif
								}
						
							/*****************ÒÑ¾­²úÉúµÄĞĞ¶¯²½Öè²»ÄÜ±»ÆäËû×¢²áÇø²úÉúµÄĞĞ¶¯²½ÖèÆÆ»µµô*******************/
						WaitDataAnalysisOver_Flag=1;
					/************************************************/

							tastate=TA_OOM_Init;
						#if	defined (USE_MC_DEBUG) && defined (PRINTF_DEBUG)		
										printf("\r\n  WAITDATABUF[%d] done a little bit ,the waitdata type is %2x,the supposed receive data quantity is %d\r\n"
													,wdr_bufnum,WAITDATABUF[wdr_bufnum]->command_type,WAITDATABUF[wdr_bufnum]->WaitDataResponse_Quantity);
						#endif
					}
					/***********µÈ´ıÊ±ÏŞÄÚÃ»ÊÕµ½ÏëÒªµÄÊı¾İ£¬ÄÇ¾Í¼ÌĞøÈ¥×öÆäËûÊÂÇé*******************************************/
				else
					{tastate=TsakSW_Analy;}
			}
		/************************ÒÑ¾­²»ÔÚ½ÓÊÕÊ±ÏŞÄÚÁË,Çå³ıµÈ´ıÊı¾İµÄ±êÖ¾£¬Í¬Ê±Ìø×ª±¸ÓÃĞĞ¶¯²½Öè********************************/
		else
			{
						#if	defined (USE_MC_DEBUG) && defined (PRINTF_DEBUG)		
							printf("\r\n  time out ,MCU have been waiting for %d timer2 interrupt \r\n",waitdata_time[wdr_bufnum]);
						#endif
				WAITDATABUF[wdr_bufnum]->WaitDataFlag=0;
				Com_Param=WAITDATABUF[wdr_bufnum]->comparam_copy;
				Aim_Param=WAITDATABUF[wdr_bufnum]->aimparam_copy;
				step_num=WAITDATABUF[wdr_bufnum]->step_num_copy+4;
				
				/*****************ÒÑ¾­²úÉúµÄĞĞ¶¯²½Öè²»ÄÜ±»ÆäËû×¢²áÇø²úÉúµÄĞĞ¶¯²½ÖèÆÆ»µµô*******************/
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
			if(WAITDATABUF[i]->WaitDataFlag == 2)//ĞèÒªµÈ´ıÁ¢¼´ÏìÓ¦
				{ImeAck_Handle(i);}
			else if(WAITDATABUF[i]->WaitDataFlag == 1) 
				{
					 if(WAITDATABUF[i]->noneed_dataresponse == 1)//²»ĞèÒªµÈ´ıÊı¾İ
						{WaitDataRegister_Clear(i);}
					 else if(WAITDATABUF[i]->noneed_dataresponse == 2) //²»ĞèÒªµÈ´ıÊı¾İµ«ÊÇ¿ÉÄÜĞèÒªµÈ´ıÊ§Áª±¨¸æ
						{Terminal_LoseReport_Handle(i);}
					 else if (WAITDATABUF[i]->noneed_dataresponse == 3)//ĞèÒªµÈ´ıÊı¾İ»òÕßÊ§Áª±¨¸æ
						{
							if(WAITDATABUF[i]->WaitDataResponse_Quantity >= 2 )//ĞèÒªµÈ´ı¶à¸öÊı¾İ»òÕßÒ»¸öÊ§Áª±¨¸æ
								{
									WAITDATABUF[i]->waitdata_timelimit=300;//¿¼ÂÇµ½Æµ·±·¢Êı¾İÎŞÏß´®¿ÚÃ¦²»¹ıÀ´£¬ÒÔ¼°Ä¿µÄÖÕ¶ËÊ§ÁªµÄÇé¿ö£¬ÕâÀï°ÑµÈ´ıÊ±ÏŞ´Ó2ÃëÉı¸ßµ½15Ãë
									WaitMoreThanOneData_Handle(i,1);
								}
								else //´ËÊ±WaitDataResponse_QuantityÖ»¿ÉÄÜµÈÓÚ1.ĞèÒªµÈ´ıÒ»¸öÊı¾İ»òÕßÒ»¸öÊ§Áª±¨¸æ
								{WaitOneData_Handle(i,1);}	
						}
					 else //ĞèÒªµÈ´ıÊı¾İ£¬´ËÊ±noneed_dataresponse Ö»¿ÉÄÜµÈÓÚ0
						{
								if(WAITDATABUF[i]->WaitDataResponse_Quantity >= 2 )//ĞèÒªµÈ´ı¶à¸öÊı¾İ
								{
									WAITDATABUF[i]->waitdata_timelimit=100;//¿¼ÂÇµ½Æµ·±·¢Êı¾İÎŞÏß´®¿ÚÃ¦²»¹ıÀ´£¬ÕâÀï°ÑµÈ´ıÊ±ÏŞ´Ó2ÃëÉı¸ßµ½5Ãë
									WaitMoreThanOneData_Handle(i,0);
								}
								else //´ËÊ±WaitDataResponse_QuantityÖ»¿ÉÄÜµÈÓÚ1.
								{WaitOneData_Handle(i,0);}
						}
				}
			else {tastate=TsakSW_Analy;}
			
			if(WaitDataAnalysisOver_Flag==0) {i++;}
			else {i=4;}//ÓÃÓÚÖ±½ÓÌø³öFORÑ­»·
	
	}
}

/*********************************************************
´ÓÌØ¶¨µØÖ·¿ªÊ¼ÏòÇ°¶ÁÈ¡flashÖĞµÄÌØ¶¨ÊıÁ¿µÄÊı¾İ¡£
ÒòÎªÕâ¸öº¯ÊıµÄÖ´ĞĞÊ±¼äÈ¡¾öÓÚÖ¸ÁîÖĞµÄÊ±¼ä¿ç¶È£¬ËùÒÔÔÚº¯ÊıÖĞ¼ÓÈëÎ¹¿´ÃÅ¹·µÄ²Ù×÷¡£
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
			
	 //µ½´Ë´¦£¬the_earlist_addrÇ°ÃæµÄÒ»¸öÉÈÇøÒÑ¾­±»²Á³ıÖØĞÂĞ´ÈëÁË¡£
			
	 addr_two_copy=addr_two;
	
	 if(datapack_num1 == 0) //²»¿ÉÄÜÓĞÕâÖÖÇé¿ö
		{
												 #if	defined (USE_MC_DEBUG) && defined (PRINTF_DEBUG)		
														printf("\r\n something wrong ! datapack_num1=0 \r\n");
													#endif
			return 0;
		}
	 else
		{  
		   for(i=0;i<datapack_num1;i++)//datapack_num1µÄÈ¡ÖµÖ»¿ÉÄÜÊÇ£º1,2,3.......PackNum_In_Section
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
						i=datapack_num1;//Ïàµ±ÓÚbreak;ÓÃÓÚÌø³öµ±Ç°forÑ­»·
						section_num=0;//ÕâÑùºóÃæµÄforÑ­»·Ò²²»»á½øÈë
						datapack_num2=0;//ÕâÑùºóÃæµÄforÑ­»·Ò²²»»á½øÈë
					}
					if(addr_in_certain_section % 4096 == 0)
					{
					  i=datapack_num1;//Ïàµ±ÓÚbreak;ÓÃÓÚÌø³öµ±Ç°forÑ­»·
					}
				}
				
				addr_two_copy=addr_in_certain_section;
				IWDG_Feed();
				//´ËÊ±section_numµÄÈ¡ÖµÖ»¿ÉÄÜÊÇ£º0,1,2......FlashValidData_MaxSectionNum
				//´ËÊ±addr_two_copyµÄÈ¡ÖµÖ»¿ÉÄÜÊÇsection_num*4096.
				
				
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
////////////								  if(addr_in_certain_section == the_earlist_addr)//ÒÑ¾­µ½´ïÁË×îÔçÊı¾İ£¬¸Ã½áÊøÁË
////////////									{
////////////									   j=PackNum_In_Section;//Ïàµ±ÓÚbreak;Ìø³öµ±Ç°forÑ­»·
////////////										 i=section_num;//Ïàµ±ÓÚbreak;Ìø³öÍâ²ãforÑ­»·
////////////										 datapack_num2=0;//ÕâÑùºóÃæµÄforÑ­»·Ò²²»»á½øÈë
////////////									}
							 }	
							 
							 if(addr_two_copy == the_earlist_addr)
							 {
							   i=section_num;//Ïàµ±ÓÚbreak;Ìø³öforÑ­»·
								 datapack_num2=0;//ÕâÑùºóÃæµÄforÑ­»·Ò²²»»á½øÈë
							 }
					IWDG_Feed();
					}

					
				//´ËÊ±addr_two_copyµÄÈ¡ÖµÖ»¿ÉÄÜÊÇsection_num*4096.
				if(addr_two_copy >=4096 ) 
					{addr_two_copy=addr_two_copy-4096;}
				else //addr_two_copy=0;
					{addr_two_copy=(FlashValidData_MaxSectionNum<<12);}
				for(i=0;i<datapack_num2;i++)//datapack_num2µÄÈ¡ÖµÖ»¿ÉÄÜÊÇ£º1,2,3.......PackNum_In_Section
					{
							addr_in_certain_section=addr_two_copy+(PackNum_In_Section-1-i)*FlashPackData_Length;
					    df_read_open(addr_in_certain_section);
							df_read(&SendBufThree[8],FlashPackData_Length);
							Usart_send(SendBufThree,FlashPackData_Length+10,COMMANDBUF[ComBufNum]->com_from,1,1);
													#if	defined (USE_MC_DEBUG) && defined (PRINTF_DEBUG)		
														printf("\r\n the data pack from addr %d transup \r\n",addr_in_certain_section);
													#endif
						  if(addr_in_certain_section == the_earlist_addr)//ÒÑ¾­µ½´ïÁË×îÔçÊı¾İ£¬¸Ã½áÊøÁË
									{
									   i=datapack_num2;//Ïàµ±ÓÚbreak;Ìø³öµ±Ç°forÑ­»·
									}
					}
			IWDG_Feed();
			return 1;
		}

	
	
}



/*********************************************************
´ÓÌØ¶¨µØÖ·¿ªÊ¼Ïòºó¶ÁÈ¡flashÖĞµÄÌØ¶¨ÊıÁ¿µÄÊı¾İ¡£
ÒòÎªÕâ¸öº¯ÊıµÄÖ´ĞĞÊ±¼äÈ¡¾öÓÚÖ¸ÁîÖĞµÄÊ±¼ä¿ç¶È£¬ËùÒÔÔÚº¯ÊıÖĞ¼ÓÈëÎ¹¿´ÃÅ¹·µÄ²Ù×÷¡£
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
			
	 //µ½´Ë´¦£¬the_earlist_addrÇ°ÃæµÄÒ»¸öÉÈÇøÒÑ¾­±»²Á³ıÖØĞÂĞ´ÈëÁË¡£
			
	 addr_two_copy=addr_two;
	
	 if(datapack_num1 == 0) //²»¿ÉÄÜÓĞÕâÖÖÇé¿ö
		{
												 #if	defined (USE_MC_DEBUG) && defined (PRINTF_DEBUG)		
														printf("\r\n something wrong ! datapack_num1=0 \r\n");
													#endif
			return 0;
		}
	 else
		{  
		   for(i=0;i<datapack_num1;i++)//datapack_num1µÄÈ¡ÖµÖ»¿ÉÄÜÊÇ£º1,2,3.......PackNum_In_Section
				{
					addr_in_certain_section=addr_two_copy-FlashPackData_Length*i;
					datapack_num1_copy=i+1;
					addr_one=addr_in_certain_section;
					if(addr_in_certain_section == the_earlist_addr)
					{
						addr_one=the_earlist_addr;
						section_num_copy=0;
						datapack_num2_copy=0;
						
						i=datapack_num1;//Ïàµ±ÓÚbreak;ÓÃÓÚÌø³öµ±Ç°forÑ­»·
						section_num=0;//ÕâÑùºóÃæµÄforÑ­»·Ò²²»»á½øÈë
						datapack_num2=0;//ÕâÑùºóÃæµÄforÑ­»·Ò²²»»á½øÈë
					}
					if(addr_in_certain_section % 4096 == 0)
					{
					  i=datapack_num1;//Ïàµ±ÓÚbreak;ÓÃÓÚÌø³öµ±Ç°forÑ­»·
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
								  if(addr_in_certain_section == the_earlist_addr)//ÒÑ¾­µ½´ïÁË×îÔçÊı¾İ£¬¸Ã½áÊøÁË
									{
										 section_num_copy=section_num_copy-1;
										 datapack_num2_copy=j+1;
										 addr_one=the_earlist_addr;
									   j=PackNum_In_Section;//Ïàµ±ÓÚbreak;Ìø³öµ±Ç°forÑ­»·
										 i=section_num;//Ïàµ±ÓÚbreak;Ìø³öÍâ²ãforÑ­»·
										 datapack_num2=0;//ÕâÑùºóÃæµÄforÑ­»·Ò²²»»á½øÈë
									}
							 }	
							 
					}

					
				//´ËÊ±addr_two_copyµÄÈ¡ÖµÖ»¿ÉÄÜÊÇsection_num*4096.
				if(addr_two_copy >=4096 ) 
					{addr_two_copy=addr_two_copy-4096;}
				else //addr_two_copy=0;
					{addr_two_copy=(FlashValidData_MaxSectionNum<<12);}
				for(i=0;i<datapack_num2;i++)//datapack_num2µÄÈ¡ÖµÖ»¿ÉÄÜÊÇ£º1,2,3.......PackNum_In_Section
					{
							addr_in_certain_section=addr_two_copy+(PackNum_In_Section-1-i)*FlashPackData_Length;
						  addr_one=addr_in_certain_section;
						  datapack_num2_copy=i+1;
						
						  if(addr_in_certain_section == the_earlist_addr)//ÒÑ¾­µ½´ïÁË×îÔçÊı¾İ£¬¸Ã½áÊøÁË
									{
										 
										 addr_one=the_earlist_addr;
									   i=datapack_num2;//Ïàµ±ÓÚbreak;Ìø³öµ±Ç°forÑ­»·
									}
					}

	      IWDG_Feed(); 	
				addr_one=Flashaddr_Calibration(addr_one,0);//Õı³£Çé¿öÏÂaddr_oneÊÇ×¼È·µØÖ·²»ĞèÒªÕâ¸öĞ£×¼
/********************************************************************************************
½ÓÏÂÀ´ÒÔaddr_oneÎªÆğµã£¬·Ö±ğÏòºóÈ¡datapack_num2_copy¸öÊı¾İ°ü£¬section_num_copy¸öÉÈÇø£¬datapack_num1_copy¸öÊı¾İ°üµÄÊı¾İ
*************************************************************************************/
							#if	defined (USE_MC_DEBUG) && defined (PRINTF_DEBUG)		
								printf("\r\n get flash data from addr %d ,%d datapack,%d section,%d datapack backwards\r\n"
								,addr_one,datapack_num2_copy,section_num_copy,datapack_num1_copy);
							#endif		
					
			for(i=0;i<datapack_num2_copy;i++)//datapack_num2_copyµÄÈ¡ÖµÖ»¿ÉÄÜÊÇ£º1,2,3.......PackNum_In_Section
					{
						addr_in_certain_section=addr_one+FlashPackData_Length*i;
						df_read_open(addr_in_certain_section);
						df_read(&SendBufThree[8],FlashPackData_Length);
						Usart_send(SendBufThree,FlashPackData_Length+10,COMMANDBUF[ComBufNum]->com_from,1,1);
														#if	defined (USE_MC_DEBUG) && defined (PRINTF_DEBUG)		
															printf("\r\n the data pack from addr %d transup \r\n",addr_in_certain_section);
														#endif			
						FlashDataTransUp_Delay;//ÎªÁË·ÀÖ¹ÉÏÎ»»úÕ³°ü
					}
					
			addr_one=addr_one-(addr_one%4096);
					
			if(datapack_num2_copy == 0 && section_num_copy > 0) //addr_oneËùÔÚÉÈÇøÊı¾İ»¹Î´ÉÏ´«£¬Òª°Ñaddr_oneÏòÇ°ÒÆ¶¯Ò»¸öÉÈÇøÔÙ½øÈëÏÂÒ»¸öÉÈÇø·¢ËÍµÄfor Ñ­»·
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
							 FlashDataTransUp_Delay;//ÎªÁË·ÀÖ¹ÉÏÎ»»úÕ³°ü
						 }	
						IWDG_Feed(); 
				}
////////////			if(section_num_copy == 0 && datapack_num2_copy == 0) //addr_oneÒ»¶¨ÊÇÉÈÇøÆğÊ¼µØÖ·£¬¶øÇÒ¸ÃÉÈÇøµÄÊı¾İÎ´±»¶ÁÈ¡¹ı£¬ÕâÀïÊ²Ã´Ò²²»ÓÃ¸É
////////////				{
////////////				
////////////				}
////////////			else if (section_num_copy == 0 && datapack_num2_copy > 0)//addr_oneÒ»¶¨ÊÇÉÈÇøÆğÊ¼µØÖ·£¬¶øÇÒ¸ÃÉÈÇøµÄÊı¾İÒÑ¾­±»¶ÁÈ¡¹ıÁË£¬ÕâÀïÒª°Ñaddr_oneÏòºó¿çÔ½Ò»¸öÉÈÇø
////////////				{
////////////					if(addr_one+4096>FlashValidData_MaxAddr)
////////////						{addr_one =0;	}
////////////					else
////////////						{addr_one=addr_one+4096;}
////////////				
////////////				}
////////////			else if (section_num_copy >0 && datapack_num2_copy == 0)//addr_oneÒ»¶¨ÊÇÉÈÇøÆğÊ¼µØÖ·£¬¶øÇÒ¸ÃÉÈÇøµÄÊı¾İÒÑ¾­±»¶ÁÈ¡¹ıÁË£¬ÕâÀïÒª°Ñaddr_oneÏòºó¿çÔ½Ò»¸öÉÈÇø
////////////			  {
////////////					if(addr_one+4096>FlashValidData_MaxAddr)
////////////						{addr_one =0;	}
////////////					else
////////////						{addr_one=addr_one+4096;}
////////////			  }
////////////			else if (section_num_copy >0 && datapack_num2_copy > 0)//addr_oneÒ»¶¨ÊÇÉÈÇøÆğÊ¼µØÖ·£¬¶øÇÒ¸ÃÉÈÇøµÄÊı¾İÒÑ¾­±»¶ÁÈ¡¹ıÁË£¬ÕâÀïÒª°Ñaddr_oneÏòºó¿çÔ½Ò»¸öÉÈÇø
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
				
			for(i=0;i<datapack_num1_copy;i++)//datapack_num1_copyµÄÈ¡ÖµÖ»¿ÉÄÜÊÇ£º1,2,3.......PackNum_In_Section
				{
						addr_in_certain_section=addr_one+i*FlashPackData_Length;
						df_read_open(addr_in_certain_section);
						df_read(&SendBufThree[8],FlashPackData_Length);
						Usart_send(SendBufThree,FlashPackData_Length+10,COMMANDBUF[ComBufNum]->com_from,1,1);
												#if	defined (USE_MC_DEBUG) && defined (PRINTF_DEBUG)		
													printf("\r\n the data pack from addr %d transup \r\n",addr_in_certain_section);
												#endif
					  FlashDataTransUp_Delay;//ÎªÁË·ÀÖ¹ÉÏÎ»»úÕ³°ü

				}
					    				
			return 1;
		}

	
	
}