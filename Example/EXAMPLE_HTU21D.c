/*-----------------------------------------------------------------------
     Creator		: Morris chiou
     Sensor		: Temperature & Humidity sensor
     File Name		: SENSOR_HTU21D.c
     Function		: SENSOR_HTU21D
     Create Date	: 2017/07/17
---------------------------------------------------------------------- */


#include <stdio.h>
#include <math.h>
#include <delay.h>
#include <alcd.h>
#include <stdlib.h>
#include <datatype_Layer.h>
#include <swi2c_Layer.h>
#include <SENSOR_HTU21D.h>


void  EXAMPLE_HTU21D(void);


void  EXAMPLE_HTU21D(void)
{
	CHAR8S status = 0;
	FLOAT temperature =0.0,humidity = 0.0;
	CHAR8U mode = 0,battery_status = 0;
  	CHAR8U lcd_char_data[2][5]={0}; 
	INT32S data;

	
		printf("-------------------- Temperature & Humidity sensor  HTU21D --------------------\r\n");

		 /* Temperature & Humidity sensor  HTU21D */
				
		/* RESET HTU21D */
		status = HTU21D_RESET();
		if(status==0)
		{ 
			printf(" RESET HTU21D success!\r\n");
		}
		else 
		{
			printf(" RESET HTU21D fail!\r\n");
		}

		delay_ms(40);	/* after RESET HTU21D -> wait 15mS */

	
		/* disable Heat IC function */
		/* mode 0 : Humidity -> 12bit  ; Temperature -> 14bit */
		status = HTU21D_INIT();
		if(status==0)
		{
			printf(" INITIAL HTU21D success!\r\n");
		}
		else 
		{
			printf(" INITIAL HTU21D fail!\r\n");
		}

		/* check battery status */
		status = HTU21D_CHECK_BATTERY_STATUS(&battery_status);
		if(status==0)
		{		
			if(battery_status==0x01)
			{	
				printf(" HTU21D Battery VDD<2.25v \r\n");
			}
			if(battery_status==0x00)
			{
				printf(" HTU21D Battery VDD>2.25v \r\n");
			}
		}
		else 		
		{
			printf(" HTU21D read battery status fail!\r\n");
		}


		/*  check mode  */
		status = HTU21D_READ_MODE(&mode);
		if(status ==0)
		{
			if(mode | HTU21D_MODE0)
			{
				printf("HTU21D is mode 0, 0x%x\r\n",mode);
			}
			else 
			{
				printf("HTU21D is NOT mode , 0x%x\r\n",mode);
			}
		}
		else 
		{
			printf(" HTU21D read mode fail!\r\n");
		}
			

		while(1)
		{
			/* read Temperature & Check CRC checksum  & Calculate final temperature */
			status = HTU21D_READ_TEMPERATURE(&temperature,mode);
			if(status!=0) 
			{
				printf(" read HTU21D temperature data fail %d\r\n",status);
			}

			/* wait 10mS */
			delay_ms(10);
			 
			/* read Humidity & Check CRC checksum  & Calculate final Humidity */	
			status = HTU21D_READ_HUMIDITY(&humidity,mode);
			if(status!=0) printf(" read HTU21D humidity data fail\r\n");

			/* print result */		
			printf("HTU21D T:%f C	H:%f %RH\r\n",temperature,humidity);	



			/* --------- Temperature bolck --------- */
			if(temperature<0)/* temperature < 0 */
			{				
				/* Temperautre */
				lcd_char_data[0][0] = (CHAR8U)(abs(temperature/100))%10;		/* use abs() */
				lcd_char_data[0][1] = (CHAR8U)(abs(temperature/10))%10;		/* use abs() */
				lcd_char_data[0][2] = (CHAR8U)(abs(temperature))%10;			/* use abs() */  
				lcd_char_data[0][3] = (CHAR8U)(abs(temperature*10))%10;		/* use abs() */   
				lcd_char_data[0][4] = (CHAR8U)(abs(temperature*100))%10; 		/* use abs() */  		
			}
			else /* temperature >= 0 */
			{
				/* Temperautre */
				lcd_char_data[0][0] = (CHAR8U)(temperature/100)%10;     
				lcd_char_data[0][1] = (CHAR8U)(temperature/10)%10;
				lcd_char_data[0][2] = (CHAR8U)(temperature)%10;  
				lcd_char_data[0][3] = (CHAR8U)(temperature*10)%10;   
				lcd_char_data[0][4] = (CHAR8U)(temperature*100)%10;   						
			}
				
			/* SHOW Temperautre */                    
	            	lcd_gotoxy(0,0);
	            	lcd_putsf("T:");     
	            	if(temperature<0)
	            	{
	            		lcd_putchar(45);		/* LCD show "-"  mean negative */
			}                             
	           	else
			{
				lcd_putchar(32);		/* LCD show " "  mean positive */
			}   
					
			/* show Temperautre data on LCD */
			lcd_putchar(48+lcd_char_data[0][0]);
			lcd_putchar(48+lcd_char_data[0][1]);
			lcd_putchar(48+lcd_char_data[0][2]);
			lcd_putchar(46);    /* print "."  */					
			lcd_putchar(48+lcd_char_data[0][3]);
			lcd_putchar(48+lcd_char_data[0][4]);
			lcd_putsf("C");
			/* --------- Temperature bolck --------- */

			
			/* --------- Humidity bolck --------- */
	              /* Humidity */
			lcd_char_data[0][0] = (INT32U)(humidity/100)%10;     
			lcd_char_data[0][1] = (INT32U)(humidity/10)%10;
			lcd_char_data[0][2] = (INT32U)(humidity)%10;  
			lcd_char_data[0][3] = (INT32U)(humidity*10)%10;   
			lcd_char_data[0][4] = (INT32U)(humidity*100)%10;   	                   
				
			/* SHOW Humidity */                    
	            	lcd_gotoxy(0,1);
	            	lcd_putsf("H:");     

			/* show Humidity data on LCD */
			lcd_putchar(48+lcd_char_data[0][0]);
			lcd_putchar(48+lcd_char_data[0][1]);
			lcd_putchar(48+lcd_char_data[0][2]);
			lcd_putchar(46);    /* print "."  */					
			lcd_putchar(48+lcd_char_data[0][3]);
			lcd_putchar(48+lcd_char_data[0][4]);
			lcd_putsf("%RH");
			/* --------- Humidity bolck --------- */

			/* --------- Display ID bolck --------- */
			/* SHOW IC ID */                    
	            	lcd_gotoxy(0,2);
	            	lcd_putsf("HTU21D");    
			/* --------- Display ID bolck --------- */

				
			/* Delay */
			delay_ms(200);
				
		}
			

		printf("-------------------- Temperature & Humidity sensor  HTU21D --------------------\r\n");
	  
}

