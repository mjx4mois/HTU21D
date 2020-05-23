/*-----------------------------------------------------------------------
     Creator		: Morris chiou
     Sensor		: Temperature & Humidity sensor
     File Name		: SENSOR_HTU21D.h
     Function		: SENSOR_HTU21D
     Create Date	: 2017/07/13
---------------------------------------------------------------------- */
#ifndef __HTU21D_HEADER__ 
#define __HTU21D_HEADER__  

/*--------------------------------- Define SLAVE ADDRESS -------------------------------------*/
// HTU21D SLAVE ADDRESS
#define HTU21D_SLAVE_ADDRESS 				0x80	//the Same Slv Addr with SHT20 , SI7021
/*--------------------------------- Define SLAVE ADDRESS -------------------------------------*/


/* ------------- DEFINE HTU21D REGISTER  ------------*/
/* USE NO HOLD MASTER REGISTER ! --> if CHANGE to HOLD MASTER see SPEC!*/
//#define HTU21D_HOLD_MASTER_TEMP			0xE3
//#define HTU21D_HOLD_MASTER_HUM			0xE5
#define HTU21D_NO_HOLD_MASTER_TEMP		0xF3
#define HTU21D_NO_HOLD_MASTER_HUM		0xF5
#define HTU21D_WRITE_USER_REG				0xE6
#define HTU21D_READ_USER_REG				0xE7
/*
	resolution set   (def:00)
	bit 7		bit0		RH			Temp		   mode*
	0		0		12bit		14bit			0
	0		1		8bit			12bit			1		
	1		0		10bit		13bit			2
	1		1		11bit		11bit			3
	--------------------------
	bit 6  End of Battery (def:0) ;  if 0 ->  [VDD>2.25v] ; if 1 -> [VDD<2.25v]
	--------------------------
	bit 5,4,3 Reserved	(def:0)
	--------------------------
	bit 2 Enable on-chip header !(def:0)   
	Note : OTP reload active loads default setting after each time a measurement command is issued.
	--------------------------
	bit 1 Disable OTP reload	 (def:1)	
*/																
#define HTU21D_SOFT_RST						0xFE		/*after command -> 15mS wait sensor reset ok!*/
/*------------- DEFINE HTU21D REGISTER  ------------*/




/*------------- DEFINE HTU21D STRUCT ------------*/

#define HTU21D_USER_REG_DEFAULT 0x02

/*--------------------------------*/
/*
	SET BIT7 & BIT 0
	read Temperature & Humidity
	RH   	      Temp	mode*			bit7		bit0
	12bit		14bit		0			0		0
	8bit			12bit		1			0		1
	10bit		13bit		2			1		0
	11bit		11bit		3			1		1
	return 2 value Temperature & Humidity
	check mode to know the resolution
*/
typedef enum{
		 HTU21D_MODE0 = 0x00,				
		 HTU21D_MODE1 = 0X01,		
		 HTU21D_MODE2 = 0X80,
		 HTU21D_MODE3 = 0X81
}HTU21D_MODE;	
/*--------------------------------*/

/*--------------------------------*/
/*---------------------------*/
/*
	RH  	 	
		resolution         time
	      	12bit			16ms
	  	  8bit			3ms
         	10bit			5ms
         	11bit			8ms
*/
/*---------------------------*/
/*	 Temp 
		resolution         time
       	14bit			50ms
	  	13bit			25ms
         	12bit			13ms
         	11bit			7ms
*/
/*---------------------------*/	
typedef enum{
		HTU21D_RH_12BIT_DELAY	= 16,
		HTU21D_RH_8BIT_DELAY		= 3,
		HTU21D_RH_10BIT_DELAY	= 5,
		HTU21D_RH_11BIT_DELAY	= 11,
		HTU21D_T_14BIT_DELAY		= 50,
		HTU21D_T_13BIT_DELAY		= 25,
		HTU21D_T_12BIT_DELAY		= 13,
		HTU21D_T_11BIT_DELAY		= 7
}HTU21D_DELAY;
/*--------------------------------*/
/* ------------- DEFINE HTU21D STRUCT ------------*/



//********************************************* SYSTEM *************************************************
/*--------------------------------------------------------------------------------------------------*/
/* initial HTU21D */
/* -> set Temperature/Humidity resoluction   ; default mode 0  // Humidity -> 12bit  ; Temperature -> 14bit */
/* disable Heat IC function */
CHAR8S HTU21D_INIT(void);
/*--------------------------------------------------------------------------------------------------*/
/*--------------------------------------------------------------------------------------------------*/
/* HTU21D IC WRITE command */
CHAR8S HTU21D_WRITE_COMMAND(CHAR8U command);
/*--------------------------------------------------------------------------------------------------*/
/*--------------------------------------------------------------------------------------------------*/
/* HTU21D IC READ command */
CHAR8S HTU21D_READ_COMMAND(CHAR8U *r_data_stream);
/*--------------------------------------------------------------------------------------------------*/
/*--------------------------------------------------------------------------------------------------*/
/*
	reset HTU21D IC
	Note : delay 15mS after set reset . let HTU21D initial!
		   special I2C protocol
		   write command 0xFE to reset.
*/
CHAR8S HTU21D_RESET(void);
/*--------------------------------------------------------------------------------------------------*/
/*--------------------------------------------------------------------------------------------------*/
/*
	read Temperature 
		RH			Temp		mode*
		12bit		14bit		0
		8bit			12bit		1		
		10bit		13bit		2
		11bit		11bit		3
	check mode to know the resolution
	MEASUREMENT TIME
	Temp  	
		resolution         time
		14bit		50ms
	 	13bit		25ms
	      	12bit		13ms
	      	11bit		7ms
	---------------------------
	bit 7   bit0		RH   	  	Temp		   mode*
	0       0		12bit		14bit			0
	0       1		8bit			12bit			1		
	1       0		10bit		13bit			2
	1       1		11bit		11bit			3
	--------------------------
//**** calculate equation ****
*/
/* TEMPERATURME FORMULA =  -46.85 + 175.72 *( sample_T / 2^(sample bit)  ) , sample bit = resolution bit   , sample_RH = Read Value*/
CHAR8S HTU21D_READ_TEMPERATURE(FLOAT *t,CHAR8U mode)	;		
/*--------------------------------------------------------------------------------------------------*/
/*--------------------------------------------------------------------------------------------------*/
/*
	read Humidity
	RH			Temp		mode*
	12bit		14bit		0
	8bit			12bit		1		
	10bit		13bit		2
	11bit		11bit		3
	check mode to know the resolution
	MEASUREMENT TIME
	RH  	 	
	resolution         time
	12bit		16ms
	8bit			3ms
	10bit		5ms
	11bit		8ms
	---------------------------
	bit 7   bit0		RH			Temp		   mode*
	0       0		12bit		14bit			0
	0       1		8bit			12bit			1		
	1       0		10bit		13bit			2
	1       1		11bit		11bit			3
	--------------------------
	**** calculate equation ****
*/
/* HUMIDITY FORMULA = -6 + 125 *( sample_RH  / 2^(sample bit)   )   , sample bit = resolution bit   , sample_RH = Read Value*/
CHAR8S HTU21D_READ_HUMIDITY(FLOAT *h,CHAR8U mode);
/*--------------------------------------------------------------------------------------------------*/
/*--------------------------------------------------------------------------------------------------*/
/* use HTU21D check the battery(power) voltage  -> USER REG 0XE7 bit6*/
/* status  if 0x00 ->  [VDD>2.25v] ; if 0x01 -> [VDD<2.25v]*/
CHAR8S HTU21D_CHECK_BATTERY_STATUS(CHAR8U *status);
/*--------------------------------------------------------------------------------------------------*/
/*--------------------------------------------------------------------------------------------------*/
/* read / write User Register data */
/* RW = 1 -> READ USER REG ; 0 -> WRITE USER REG*/
CHAR8S HTU21D_RW_USER_REG(CHAR8U rw,CHAR8U *data);
/*--------------------------------------------------------------------------------------------------*/
/*--------------------------------------------------------------------------------------------------*/
/* read mode from REG 0XE7*/
/* RESOLUTION -> set mode0 ~ mode3*/
/* **** use HTU21D_MODE enum *****/
CHAR8S HTU21D_READ_MODE(CHAR8U *mode);
/*--------------------------------------------------------------------------------------------------*/
/* write mode from REG 0XE6*/
/* RESOLUTION -> set mode0 ~ mode3*/
/* **** use HTU21D_MODE enum *****/
CHAR8S HTU21D_WRITE_MODE(CHAR8U mode);
/*--------------------------------------------------------------------------------------------------*/
/*--------------------------------------------------------------------------------------------------*/
/* let HTU21D ""enable"" on-chip heater -> USER REG bit2*/
CHAR8S HTU21D_HEAT_IC(void);
/*--------------------------------------------------------------------------------------------------*/
/*--------------------------------------------------------------------------------------------------*/
/* let HTU21D ""disable"" on-chip heater -> USER REG bit2*/
CHAR8S HTU21D_DIS_HEAT_IC(void);
/*--------------------------------------------------------------------------------------------------*/
/*--------------------------------------------------------------------------------------------------*/
/*
	 HTU21D CRC checksum 
	 Function :  x^8  + x^5 + x^4 + 1		-> 0x0131
	 return CRC 

	** first read high byte -> second read low byte
	check CRC function
	 [1bytes]   input 0xDC		->  output 0x79 	
	 [2bytes]   input 0x683A	->  output 0x7C
	 [2bytes]   input 0x4E85	->  output 0x6B 
*/
CHAR8S HTU21D_CRC_CHECKSUM(CHAR8U *data, CHAR8U bytes,CHAR8U checksum);
/*--------------------------------------------------------------------------------------------------*/
/*--------------------------------------------------------------------------------------------------*/
/* HTU21D wait delay*/
/* wait measurement time*/
void HTU21D_WAIT(CHAR8U wait_time);
/*--------------------------------------------------------------------------------------------------*/
//********************************************* SYSTEM *************************************************

#endif 		//#ifndef __HTU21D_HEADER__ 
