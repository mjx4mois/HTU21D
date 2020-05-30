/*-----------------------------------------------------------------------
     Creator		: Morris chiou
     Sensor		: Temperature & Humidity sensor
     File Name		: SENSOR_HTU21D.c
     Function		: SENSOR_HTU21D
     Create Date	: 2017/07/13
---------------------------------------------------------------------- */

#include <stdio.h>
#include <math.h>
#include <delay.h>
#include <i2c.h>
#include <datatype_Layer.h>
#include <swi2c_Layer.h>
#include <SENSOR_HTU21D.h>


/********************************************** SYSTEM **************************************************/
/*--------------------------------------------------------------------------------------------------*/
/* 
	initial HTU21D 
	-> set Temperature/Humidity resoluction   ; default mode 0  // Humidity -> 12bit  ; Temperature -> 14bit 
	disable Heat IC function
*/
CHAR8S HTU21D_INIT(void)
{
	CHAR8S status = 0;

		status = HTU21D_WRITE_MODE(HTU21D_MODE0) ; /* mode 0 , Humidity -> 12bit  ; Temperature -> 14bit */
		if(status !=0)
		{
			return -1;	/*write mode fail*/
		}
		
		status = HTU21D_DIS_HEAT_IC();	/*dis HEAD IC.*/
		if(status!=0)
		{
			return -1;		/* dis HEAD IC fail.*/
		}
		
		return 0;	/*initial HTU21D success !!*/
}
/*--------------------------------------------------------------------------------------------------*/
/*--------------------------------------------------------------------------------------------------*/
/* HTU21D IC WRITE command */ 
CHAR8S HTU21D_WRITE_COMMAND(CHAR8U command)
{
	CHAR8S ack=0,busy=0;

		busy = i2c_start();
		if(busy)
		{
			ack=i2c_write( HTU21D_SLAVE_ADDRESS | WRITE_BIT);
			if(ack == 1)
			{
				ack=i2c_write(command);		
				if(ack == 1)
				{
					i2c_stop();
					delay_us(10);
					return SWI2C_STATUS_OK;
				}
				else
				{
					printf("register error\r\n");
					goto EXIT;
				}
			}
			else
			{
				printf("address error\r\n");
				goto EXIT;
			}
		}
		else
		{
			EXIT:
					i2c_stop();
					delay_us(10);
					return SWI2C_STATUS_FAIL;
		}

}
/*--------------------------------------------------------------------------------------------------*/
/*--------------------------------------------------------------------------------------------------*/
/* HTU21D IC READ command */
CHAR8S HTU21D_READ_COMMAND(CHAR8U *r_data_stream)
{

	CHAR8S ack=0,busy=0,data_number;
	CHAR8U n_byte = 3;

		if(n_byte>255)
		{
                    return SWI2C_STATUS_FAIL;	/*over max set 255 !!*/
		}

		busy = i2c_start();	
		if(busy)
		{
			ack = i2c_write(HTU21D_SLAVE_ADDRESS | READ_BIT);
			if(ack == 1)
			{

				for(data_number=0;data_number<n_byte;data_number++)
				{
					if(data_number == n_byte)
					{
						r_data_stream[data_number] = i2c_read(MASTER_NACK);
						delay_us(3);	/*a little delay.*/
						break;
					}
					else
					{
						r_data_stream[data_number] = i2c_read(MASTER_ACK);
						delay_us(3);	/* a little delay.*/
					}
				}
				
				 /* ALL read finish!! */
				i2c_stop();
				delay_us(10);
				return SWI2C_STATUS_OK;

			}
			else
			{
				printf("address error\r\n");
				goto EXIT;
			}
		}        
		else
		{
			EXIT:
			         i2c_stop();
			         delay_us(10);
				  return SWI2C_STATUS_FAIL;
		}

}
/*--------------------------------------------------------------------------------------------------*/
/*--------------------------------------------------------------------------------------------------*/
/* reset HTU21D IC*/
/* Note : 	delay 15mS after set reset . let HTU21D initial!		*/
/* 		   	special I2C protocol							*/
/* 		   	write command 0xFE to reset.					*/
/*			after command -> 15mS wait sensor reset ok		*/
CHAR8S HTU21D_RESET(void)
{
	CHAR8S status = 0;

	 status = HTU21D_WRITE_COMMAND(HTU21D_SOFT_RST);
	 if(status!=1)
	 {
	 	return -1;	/*reset fail. */
	 }
	 return 0;	/* reset success !*/
	 
}
/*--------------------------------------------------------------------------------------------------*/
/*--------------------------------------------------------------------------------------------------*/
/*	read Temperature 
		RH			Temp		mode*
		12bit		14bit			0
		8bit			12bit			1		
		10bit		13bit			2
		11bit		11bit			3
 	check mode to know the resolution
 	MEASUREMENT TIME
	Temp  	
			resolution			time
				14bit			50ms
				13bit			25ms
				12bit			13ms
				11bit			7ms
   ---------------------------
	bit 7	bit0		RH			Temp	mode*
	0		0		12bit		14bit		0
	0		1		8bit			12bit		1		
	1		0		10bit		13bit		2
	1		1		11bit		11bit		3
   --------------------------
  **** calculate equation ****
*/
/*TEMPERATURME FORMULA =  -46.85 + 175.72 *( sample_T / 2^(sample bit)  ) , sample bit = resolution bit   , sample_RH = Read Value */
CHAR8S HTU21D_READ_TEMPERATURE(FLOAT *t,CHAR8U mode)
{
	CHAR8S status = 0 ;
	CHAR8U cnt = 0,read_data[2] = {0};

	INT16U temp_data = 0;  /* unsigned? YES */	
	FLOAT temperautre = 0.0;

		/* check mode */
		if(mode !=HTU21D_MODE0  && mode !=HTU21D_MODE1 && mode !=HTU21D_MODE2 &&mode !=HTU21D_MODE3  )
		{
			return -1;	/*error mode value*/
		}

		/* command -> Temperature */ 
		status = HTU21D_WRITE_COMMAND(HTU21D_NO_HOLD_MASTER_TEMP);
		if(status!=1)
		{
			return -1;	/*  write command fail.*/
		}

		/* delay for temperature delay */
		switch(mode)
		{
			case  HTU21D_MODE0:
					HTU21D_WAIT(HTU21D_T_14BIT_DELAY);
				break;					
			case  HTU21D_MODE1:
					HTU21D_WAIT(HTU21D_T_13BIT_DELAY);
				break;					
			case  HTU21D_MODE2:
					HTU21D_WAIT(HTU21D_T_12BIT_DELAY);
				break;										
			case  HTU21D_MODE3:
					HTU21D_WAIT(HTU21D_T_11BIT_DELAY);
				break;
		}
		
		/* check & re-try measurement ok? */
		/* re-try = 4 */
		for(cnt=0;cnt<4;cnt++)
		{		
			status = HTU21D_READ_COMMAND(&read_data[0]);;
			if(status!=1)/* return NACK , HTU21D busy */
			{
				HTU21D_WAIT(5);	/* delay 5mS	; unit : 1mS */
			}
			else 
			{
				break;	/* read success!*/
			}
		}
		
		if(status!=1)
		{
			return -1; 	/* re-try 4  -> all read fail !. */
		}
	

		/* read_data[0] -> first byte MSB */
		/* read_data[1] -> second byte	LSB	// bit1 -> T/H bit[1:Humidity 0:Temperature]  ; bit0 -> reserved.*/
		/* read_data[2] -> CRC byte */

		/*check read_data[1] bit 1 = 0 ?  , if bit =1 -> humidity , 0 -> temperature*/
		if((read_data[1] & 0x02)!=0) 
		{
			return -2;	/*This is not temperature value*/
		}

		/* check CRC code */	
		status = HTU21D_CRC_CHECKSUM(&read_data[0],2,read_data[2]);
		if(status!=0)
		{
			return -3;		/* CRC code match error!!*/
		}
		
		/* calculate the temperature value.*/
		/* 16bit value , MSB << left 8*/
		temp_data = (read_data[0] << 8) |(read_data[1] & 0xFC) ; /* 0xFC mask bit 1 & bit 0 */

		
		/* TEMPERATURME FORMULA =  -46.85 + 175.72 *( sample_T / 2^(sample bit)  ) */
		temperautre = -46.85 + (175.72 * (FLOAT)(temp_data)/65536);

		*t= (FLOAT)temperautre;	/*final temperature*/
		return 0; /*finish read & calculate temperature.*/	
		
}
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
		resolution		time
		12bit			16ms
		8bit				3ms
		10bit			5ms
		11bit			8ms
   ---------------------------
 	bit7   bit0		RH			Temp		   mode*
	0       0		12bit		14bit			0
	0       1		8bit			12bit			1		
	1       0		10bit		13bit			2
	1       1		11bit		11bit			3
   --------------------------
//**** calculate equation ****
*/
/*HUMIDITY FORMULA = -6 + 125 *( sample_RH  / 2^(sample bit)   )   , sample bit = resolution bit   , sample_RH = Read Value */
CHAR8S HTU21D_READ_HUMIDITY(FLOAT *h,CHAR8U mode)
{
	CHAR8S status = 0 ;
	CHAR8U cnt = 0,read_data[2] = {0};

	INT16U humidity_data = 0;  /* unsigned? YES */
	FLOAT humidity = 0.0;

		/* check mode */
		if(mode !=HTU21D_MODE0 &&mode !=HTU21D_MODE1  && mode !=HTU21D_MODE2  && mode !=HTU21D_MODE3  )
		{
			return -1;	/*error mode value*/
		}

		/* command -> Humidity*/
		status = HTU21D_WRITE_COMMAND(HTU21D_NO_HOLD_MASTER_HUM);
		if(status!=1) 
		{
			return -1;	/*write command fail.*/
		}

		/* delay for humidity delay */
		switch(mode)
		{

			case  HTU21D_MODE0:
						HTU21D_WAIT(HTU21D_RH_12BIT_DELAY);
							break;					
			case  HTU21D_MODE1:
						HTU21D_WAIT(HTU21D_RH_8BIT_DELAY);
							break;					
			case  HTU21D_MODE2:
						HTU21D_WAIT(HTU21D_RH_10BIT_DELAY);
							break;										
			case  HTU21D_MODE3:
						HTU21D_WAIT(HTU21D_RH_11BIT_DELAY);
							break;
		}
		

		/* check & re-try measurement ok? */
		/* re-try = 4 */
		for(cnt=0;cnt<4;cnt++)
		{
			status = HTU21D_READ_COMMAND(&read_data[0]);
			if(status!=1)
			{
				HTU21D_WAIT(5);	/*delay 5mS ;  unit : 1mS*/
			}
			else 
			{
			 	break;	/* read success !*/
			}
		}

		if(status!=1)
		{
			return -1;	/* re-try 4  -> all read fail !.*/
		}
	

		/* read_data[0] -> first 	byte	MSB*/
		/* read_data[1] -> second byte	LSB	// bit1 -> T/H bit[1:Humidity 0:Temperature]  ; bit0 -> reserved.*/
		/* read_data[2] -> CRC 	byte	CRC*/

		/* check read_data[1] bit 1 = 0 ?  , if bit =1 -> humidity , 0 -> temperature*/
		if(read_data[1] & 0x02 == 1) 
		{
			return -2;	/*is not humidity value*/
		}
		
		status = HTU21D_CRC_CHECKSUM(&read_data[0],2,read_data[2]);
		if(status!=0) 
		{
			return -3;	/*CRC code match error!!*/
		}
		
		/* calculate the humidity value.*/
		/* 16bit value , MSB << left 8*/
		humidity_data = (read_data[0] << 8) |(read_data[1] & 0xFC) ; /*0xFC mask bit 1 & bit 0*/

		/* HUMIDITY FORMULA = -6 + 125 *( sample_RH  / 2^(sample bit)   )*/
		humidity = -6.0 + (125.0  * (FLOAT)(humidity_data)/65536);

		*h= (FLOAT)humidity;		/* final humidity*/
		return 0;	/* finish read & calculate humidity.*/

}
/*--------------------------------------------------------------------------------------------------*/
/*--------------------------------------------------------------------------------------------------*/
/* use HTU21D check the battery(power) voltage  -> USER REG 0XE7 bit6*/
/* status  if 0x00 ->  [VDD>2.25v] ; if 0x01 -> [VDD<2.25v]*/
CHAR8S HTU21D_CHECK_BATTERY_STATUS(CHAR8U *status)
{
	CHAR8S status_return = 0 ;
	CHAR8U data = 0 ;

		status_return = HTU21D_RW_USER_REG(0x01,&data);
		if(status_return!=0) 
		{
			return -1;	/*read command fail.*/
		}
		
		/*check bit 6*/
		if(data & 0x40)
		{
			*status = 0x01;	/*[VDD<2.25v]*/
		}
		else
		{
			*status = 0x00;	/*[VDD>2.25v]*/
		}
		
		return 0;	/*read battery status ok!*/
}
/* --------------------------------------------------------------------------------------------------*/
/* --------------------------------------------------------------------------------------------------*/
/*  read / write User Register data */
/*  RW = 1 -> READ USER REG ; 0 -> WRITE USER REG*/
CHAR8S HTU21D_RW_USER_REG(CHAR8U rw,CHAR8U *data)
{
	CHAR8S status = 0 ;
	CHAR8U temp = 0 ;
	
		if(rw !=0x00 && rw!=0x01) 
		{
			return -2; /* error rw value*/
		}
		
		if(rw == 0x01) /*read user reg*/
		{
			//i2c_stop_hang();
			status = i2c_read_1_byte_data(HTU21D_SLAVE_ADDRESS,HTU21D_READ_USER_REG,&temp);
			if(status !=1 ) 
			{	
				return -1 ;/*error read fail*/
			}
			
			*data = temp;	/*read data */
			return 0 ;	/* read success !*/		
		}

		if(rw == 0x00) /*write user reg*/
		{		
			/* 0xc7 mask bit3,4,5 as 0 . Do  AND . */
			temp = *data & 0xC7;
			//i2c_stop_hang();				
			status = i2c_write_1_byte_data(HTU21D_SLAVE_ADDRESS,HTU21D_WRITE_USER_REG,temp);
			if(status !=1 ) 
			{	
				return -1 ;	/*error read fail*/
			}
			return 0 ; 	/*write success !*/		
		}
	
}
/* --------------------------------------------------------------------------------------------------*/
/* --------------------------------------------------------------------------------------------------*/
/*  read mode from REG 0XE7*/
/*  RESOLUTION -> set mode0 ~ mode3*/
/*  **** use HTU21D_MODE enum *****/
CHAR8S HTU21D_READ_MODE(CHAR8U *mode)
{
	CHAR8S status = 0;
	CHAR8U read_mode = 0;
		
		status  = HTU21D_RW_USER_REG(0x01,&read_mode);
		if(status!=0)
		{
			return -1;
		}
		
		/* catch bit7 & bit 0*/
		read_mode &= 0x81;
		*mode = read_mode ;	/*read mode.*/
		
		return 0;	/*read mode ok!.*/
}
/* --------------------------------------------------------------------------------------------------*/
/* --------------------------------------------------------------------------------------------------*/
/*  write mode from REG 0XE6 */
/*  RESOLUTION -> set mode0 ~ mode3*/
/*  **** use HTU21D_MODE enum *****/
CHAR8S HTU21D_WRITE_MODE(CHAR8U mode)
{
	CHAR8S status = 0;
	CHAR8U write_mode = 0,read_reg = 0;
	
		if(mode != HTU21D_MODE0 && mode !=HTU21D_MODE1 && mode !=HTU21D_MODE2 && mode !=HTU21D_MODE3  )
		{
			return -1;	/*error mode value*/
		}
		
		status = HTU21D_READ_MODE(&read_reg);	
		if(status!=0) 
		{
			return -1;	/* read mode fail*/
		}

		/*mask bit 7 & bit 0 */
		read_reg&=0x7E;
		
		/*set mode*/
		write_mode = read_reg | mode ;

		status = HTU21D_RW_USER_REG(0x00,&write_mode);
		if(status!=0) 
		{
			return -1;	/* write mode fail */
		}
}
/* --------------------------------------------------------------------------------------------------*/
/* --------------------------------------------------------------------------------------------------*/
/* let HTU21D ""enable"" on-chip heater -> USER REG bit2*/
CHAR8S HTU21D_HEAT_IC(void)
{
	CHAR8S status = 0;
	CHAR8U data = 0;
			
		/* read USER REG */
		status = HTU21D_RW_USER_REG(0x01,&data);
		if(status!=0) 
		{
			return -1;/*read fail*/
		}

		/* mask bit 2*/
		data&=0xFB;
		
		/* enable on-chip heater */
		data|=0x04;

		/* write USER REG */
		status = HTU21D_RW_USER_REG(0x00,&data);
		if(status!=0)
		{
			return -1;/* write fail */	
		}
		
		return 0; /* enable heat IC ok!*/
		
}
/* --------------------------------------------------------------------------------------------------*/
/* --------------------------------------------------------------------------------------------------*/
/*  let HTU21D ""disable"" on-chip heater -> USER REG bit2*/
CHAR8S HTU21D_DIS_HEAT_IC(void)
{
	CHAR8S status = 0;
	CHAR8U data = 0;
			
		/*read USER REG */
		status = HTU21D_RW_USER_REG(0x01,&data);
		if(status!=0) 
		{
			return -1;/* read fail*/
		}

		/* mask bit 2 & disable */
		data&=0xFB;

		/* write USER REG  */
		status = HTU21D_RW_USER_REG(0x00,&data);
		if(status!=0) 
		{
			return -1;/* write fail */
		}
		
		return 0;	/*disable heat IC ok!*/
		
}
/* --------------------------------------------------------------------------------------------------*/
/* --------------------------------------------------------------------------------------------------*/
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
CHAR8S HTU21D_CRC_CHECKSUM(CHAR8U *data, CHAR8U bytes,CHAR8U checksum)
{
  	 CHAR8U crc=0,cnt,cnt2; /*cnt2 counting variable */
	
		/*calculates 8-Bit checksum with given polynomial */
		for (cnt = 0; cnt<bytes; ++cnt)
		{
			crc ^= (*data++);     
					
			for (cnt2 = 0; cnt2 < 8; ++cnt2)
			{
				if (crc & 0x80) 
				{
				 	crc = ((crc << 1) ^ 0x0131);	/* 0x0131 -> Function :  x^8  + x^5 + x^4 + 1*/
				}
				else 
				{
					crc = (crc << 1);
				}
			}
		}
		
		//printf(" HTU21D_CRC_CHECKSUM 0x%x ,0x%x\r\n",checksum,crc);		   
		if(checksum == crc) 
		{
			return 0;		/* crc match!*/
		}
		else 
		{
			return -3;		/*crc error!*/
		}
 }
/* --------------------------------------------------------------------------------------------------*/
/* --------------------------------------------------------------------------------------------------*/
/*  HTU21D wait delay*/
/*  wait measurement time*/
void HTU21D_WAIT(CHAR8U wait_time)
{
	CHAR8U	cnt = 0;

		for(cnt=0;cnt<=wait_time;cnt++)
		{
			/*** Portability function here*/  
			/*unit : ms*/
			delay_ms(1);	
		}
}
/*--------------------------------------------------------------------------------------------------*/
/********************************************** SYSTEM **************************************************/

