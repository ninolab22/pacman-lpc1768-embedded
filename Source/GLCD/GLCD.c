/****************************************Copyright (c)**************************************************                         
**
**                                 http://www.powermcu.com
**
**--------------File Info-------------------------------------------------------------------------------
** File name:			GLCD.c
** Descriptions:		Has been tested SSD1289��ILI9320��R61505U��SSD1298��ST7781��SPFD5408B��ILI9325��ILI9328��
**						HX8346A��HX8347A
**------------------------------------------------------------------------------------------------------
** Created by:			AVRman
** Created date:		2012-3-10
** Version:					1.3
** Descriptions:		The original version
**
**------------------------------------------------------------------------------------------------------
** Modified by:			Paolo Bernardi
** Modified date:		03/01/2020
** Version:					2.0
** Descriptions:		simple arrangement for screen usage
********************************************************************************************************/

/* Includes ------------------------------------------------------------------*/
#include "GLCD.h" 
#include "AsciiLib.h"
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#define yellow 0xFFE0 // Colore giallo in RGB565
/* Private variables ---------------------------------------------------------*/
static uint8_t LCD_Code;

 /*matrice livello*/ 
 
 
/* Private define ------------------------------------------------------------*/
#define  ILI9320    0  /* 0x9320 */
#define  ILI9325    1  /* 0x9325 */
#define  ILI9328    2  /* 0x9328 */
#define  ILI9331    3  /* 0x9331 */
#define  SSD1298    4  /* 0x8999 */
#define  SSD1289    5  /* 0x8989 */
#define  ST7781     6  /* 0x7783 */
#define  LGDP4531   7  /* 0x4531 */
#define  SPFD5408B  8  /* 0x5408 */
#define  R61505U    9  /* 0x1505 0x0505 */
#define  HX8346A		10 /* 0x0046 */  
#define  HX8347D    11 /* 0x0047 */
#define  HX8347A    12 /* 0x0047 */	
#define  LGDP4535   13 /* 0x4535 */  
#define  SSD2119    14 /* 3.5 LCD 0x9919 */

/*******************************************************************************
* Function Name  : Lcd_Configuration
* Description    : Configures LCD Control lines
* Input          : None
* Output         : None
* Return         : None
* Attention		 : None
*******************************************************************************/
static void LCD_Configuration(void)
{
	/* Configure the LCD Control pins */
	
	/* EN = P0.19 , LE = P0.20 , DIR = P0.21 , CS = P0.22 , RS = P0.23 , RS = P0.23 */
	/* RS = P0.23 , WR = P0.24 , RD = P0.25 , DB[0.7] = P2.0...P2.7 , DB[8.15]= P2.0...P2.7 */  
	LPC_GPIO0->FIODIR   |= 0x03f80000;
	LPC_GPIO0->FIOSET    = 0x03f80000;
}


/*******************************************************************************
* Function Name  : LCD_Send
* Description    : LCDд����
* Input          : - byte: byte to be sent
* Output         : None
* Return         : None
* Attention		 : None
*******************************************************************************/
static __attribute__((always_inline)) void LCD_Send (uint16_t byte) 
{
	LPC_GPIO2->FIODIR |= 0xFF;          /* P2.0...P2.7 Output */
	LCD_DIR(1)		   				    				/* Interface A->B */
	LCD_EN(0)	                        	/* Enable 2A->2B */
	LPC_GPIO2->FIOPIN =  byte;          /* Write D0..D7 */
	LCD_LE(1)                         
	LCD_LE(0)														/* latch D0..D7	*/
	LPC_GPIO2->FIOPIN =  byte >> 8;     /* Write D8..D15 */
}

/*******************************************************************************
* Function Name  : wait_delay
* Description    : Delay Time
* Input          : - nCount: Delay Time
* Output         : None
* Return         : None
* Return         : None
* Attention		 : None 
*******************************************************************************/
static void wait_delay(int count)
{
	while(count--);
}

/*******************************************************************************
* Function Name  : LCD_Read
* Description    : LCD������
* Input          : - byte: byte to be read
* Output         : None
* Return         : ���ض�ȡ��������
* Attention		 : None
*******************************************************************************/
static __attribute__((always_inline)) uint16_t LCD_Read (void) 
{
	uint16_t value;
	
	LPC_GPIO2->FIODIR &= ~(0xFF);              /* P2.0...P2.7 Input */
	LCD_DIR(0);		   				           				 /* Interface B->A */
	LCD_EN(0);	                               /* Enable 2B->2A */
	wait_delay(30);							   						 /* delay some times */
	value = LPC_GPIO2->FIOPIN0;                /* Read D8..D15 */
	LCD_EN(1);	                               /* Enable 1B->1A */
	wait_delay(30);							   						 /* delay some times */
	value = (value << 8) | LPC_GPIO2->FIOPIN0; /* Read D0..D7 */
	LCD_DIR(1);
	return  value;
}

/*******************************************************************************
* Function Name  : LCD_WriteIndex
* Description    : LCDд�Ĵ�����ַ
* Input          : - index: �Ĵ�����ַ
* Output         : None
* Return         : None
* Attention		 : None
*******************************************************************************/
static __attribute__((always_inline)) void LCD_WriteIndex(uint16_t index)
{
	LCD_CS(0);
	LCD_RS(0);
	LCD_RD(1);
	LCD_Send( index ); 
	wait_delay(22);	
	LCD_WR(0);  
	wait_delay(1);
	LCD_WR(1);
	LCD_CS(1);
}

/*******************************************************************************
* Function Name  : LCD_WriteData
* Description    : LCDд�Ĵ�������
* Input          : - index: �Ĵ�������
* Output         : None
* Return         : None
* Attention		 : None
*******************************************************************************/
static __attribute__((always_inline)) void LCD_WriteData(uint16_t data)
{				
	LCD_CS(0);
	LCD_RS(1);   
	LCD_Send( data );
	LCD_WR(0);     
	wait_delay(1);
	LCD_WR(1);
	LCD_CS(1);
}

/*******************************************************************************
* Function Name  : LCD_ReadData
* Description    : ��ȡ����������
* Input          : None
* Output         : None
* Return         : ���ض�ȡ��������
* Attention		 : None
*******************************************************************************/
static __attribute__((always_inline)) uint16_t LCD_ReadData(void)
{ 
	uint16_t value;
	
	LCD_CS(0);
	LCD_RS(1);
	LCD_WR(1);
	LCD_RD(0);
	value = LCD_Read();
	
	LCD_RD(1);
	LCD_CS(1);
	
	return value;
}

/*******************************************************************************
* Function Name  : LCD_WriteReg
* Description    : Writes to the selected LCD register.
* Input          : - LCD_Reg: address of the selected register.
*                  - LCD_RegValue: value to write to the selected register.
* Output         : None
* Return         : None
* Attention		 : None
*******************************************************************************/
static __attribute__((always_inline)) void LCD_WriteReg(uint16_t LCD_Reg,uint16_t LCD_RegValue)
{ 
	/* Write 16-bit Index, then Write Reg */  
	LCD_WriteIndex(LCD_Reg);         
	/* Write 16-bit Reg */
	LCD_WriteData(LCD_RegValue);  
}

/*******************************************************************************
* Function Name  : LCD_WriteReg
* Description    : Reads the selected LCD Register.
* Input          : None
* Output         : None
* Return         : LCD Register Value.
* Attention		 : None
*******************************************************************************/
static __attribute__((always_inline)) uint16_t LCD_ReadReg(uint16_t LCD_Reg)
{
	uint16_t LCD_RAM;
	
	/* Write 16-bit Index (then Read Reg) */
	LCD_WriteIndex(LCD_Reg);
	/* Read 16-bit Reg */
	LCD_RAM = LCD_ReadData();      	
	return LCD_RAM;
}

/*******************************************************************************
* Function Name  : LCD_SetCursor
* Description    : Sets the cursor position.
* Input          : - Xpos: specifies the X position.
*                  - Ypos: specifies the Y position. 
* Output         : None
* Return         : None
* Attention		 : None
*******************************************************************************/
static void LCD_SetCursor(uint16_t Xpos,uint16_t Ypos)
{
    #if  ( DISP_ORIENTATION == 90 ) || ( DISP_ORIENTATION == 270 )
	
 	uint16_t temp = Xpos;

			 Xpos = Ypos;
			 Ypos = ( MAX_X - 1 ) - temp;  

	#elif  ( DISP_ORIENTATION == 0 ) || ( DISP_ORIENTATION == 180 )
		
	#endif

  switch( LCD_Code )
  {
     default:		 /* 0x9320 0x9325 0x9328 0x9331 0x5408 0x1505 0x0505 0x7783 0x4531 0x4535 */
          LCD_WriteReg(0x0020, Xpos );     
          LCD_WriteReg(0x0021, Ypos );     
	      break; 

     case SSD1298: 	 /* 0x8999 */
     case SSD1289:   /* 0x8989 */
	      LCD_WriteReg(0x004e, Xpos );      
          LCD_WriteReg(0x004f, Ypos );          
	      break;  

     case HX8346A: 	 /* 0x0046 */
     case HX8347A: 	 /* 0x0047 */
     case HX8347D: 	 /* 0x0047 */
	      LCD_WriteReg(0x02, Xpos>>8 );                                                  
	      LCD_WriteReg(0x03, Xpos );  

	      LCD_WriteReg(0x06, Ypos>>8 );                           
	      LCD_WriteReg(0x07, Ypos );    
	
	      break;     
     case SSD2119:	 /* 3.5 LCD 0x9919 */
	      break; 
  }
}

/*******************************************************************************
* Function Name  : LCD_Delay
* Description    : Delay Time
* Input          : - nCount: Delay Time
* Output         : None
* Return         : None
* Return         : None
* Attention		 : None
*******************************************************************************/
static void delay_ms(uint16_t ms)    
{ 
	uint16_t i,j; 
	for( i = 0; i < ms; i++ )
	{ 
		for( j = 0; j < 1141; j++ );
	}
} 


/*******************************************************************************
* Function Name  : LCD_Initializtion
* Description    : Initialize TFT Controller.
* Input          : None
* Output         : None
* Return         : None
* Attention		 : None
*******************************************************************************/
void LCD_Initialization(void)
{
	uint16_t DeviceCode;
	
	LCD_Configuration();
	delay_ms(100);
	DeviceCode = LCD_ReadReg(0x0000);		/* ��ȡ��ID	*/	
	
	if( DeviceCode == 0x9325 || DeviceCode == 0x9328 )	
	{
		LCD_Code = ILI9325;
		LCD_WriteReg(0x00e7,0x0010);      
		LCD_WriteReg(0x0000,0x0001);  	/* start internal osc */
		LCD_WriteReg(0x0001,0x0100);     
		LCD_WriteReg(0x0002,0x0700); 	/* power on sequence */
		LCD_WriteReg(0x0003,(1<<12)|(1<<5)|(1<<4)|(0<<3) ); 	/* importance */
		LCD_WriteReg(0x0004,0x0000);                                   
		LCD_WriteReg(0x0008,0x0207);	           
		LCD_WriteReg(0x0009,0x0000);         
		LCD_WriteReg(0x000a,0x0000); 	/* display setting */        
		LCD_WriteReg(0x000c,0x0001);	/* display setting */        
		LCD_WriteReg(0x000d,0x0000); 			        
		LCD_WriteReg(0x000f,0x0000);
		/* Power On sequence */
		LCD_WriteReg(0x0010,0x0000);   
		LCD_WriteReg(0x0011,0x0007);
		LCD_WriteReg(0x0012,0x0000);                                                                 
		LCD_WriteReg(0x0013,0x0000);                 
		delay_ms(50);  /* delay 50 ms */		
		LCD_WriteReg(0x0010,0x1590);   
		LCD_WriteReg(0x0011,0x0227);
		delay_ms(50);  /* delay 50 ms */		
		LCD_WriteReg(0x0012,0x009c);                  
		delay_ms(50);  /* delay 50 ms */		
		LCD_WriteReg(0x0013,0x1900);   
		LCD_WriteReg(0x0029,0x0023);
		LCD_WriteReg(0x002b,0x000e);
		delay_ms(50);  /* delay 50 ms */		
		LCD_WriteReg(0x0020,0x0000);                                                            
		LCD_WriteReg(0x0021,0x0000);           
		delay_ms(50);  /* delay 50 ms */		
		LCD_WriteReg(0x0030,0x0007); 
		LCD_WriteReg(0x0031,0x0707);   
		LCD_WriteReg(0x0032,0x0006);
		LCD_WriteReg(0x0035,0x0704);
		LCD_WriteReg(0x0036,0x1f04); 
		LCD_WriteReg(0x0037,0x0004);
		LCD_WriteReg(0x0038,0x0000);        
		LCD_WriteReg(0x0039,0x0706);     
		LCD_WriteReg(0x003c,0x0701);
		LCD_WriteReg(0x003d,0x000f);
		delay_ms(50);  /* delay 50 ms */		
		LCD_WriteReg(0x0050,0x0000);        
		LCD_WriteReg(0x0051,0x00ef);   
		LCD_WriteReg(0x0052,0x0000);     
		LCD_WriteReg(0x0053,0x013f);
		LCD_WriteReg(0x0060,0xa700);        
		LCD_WriteReg(0x0061,0x0001); 
		LCD_WriteReg(0x006a,0x0000);
		LCD_WriteReg(0x0080,0x0000);
		LCD_WriteReg(0x0081,0x0000);
		LCD_WriteReg(0x0082,0x0000);
		LCD_WriteReg(0x0083,0x0000);
		LCD_WriteReg(0x0084,0x0000);
		LCD_WriteReg(0x0085,0x0000);
		  
		LCD_WriteReg(0x0090,0x0010);     
		LCD_WriteReg(0x0092,0x0000);  
		LCD_WriteReg(0x0093,0x0003);
		LCD_WriteReg(0x0095,0x0110);
		LCD_WriteReg(0x0097,0x0000);        
		LCD_WriteReg(0x0098,0x0000);  
		/* display on sequence */    
		LCD_WriteReg(0x0007,0x0133);
		
		LCD_WriteReg(0x0020,0x0000);  /* ����ַ0 */                                                          
		LCD_WriteReg(0x0021,0x0000);  /* ����ַ0 */     
	}

    delay_ms(50);   /* delay 50 ms */	
}

/*******************************************************************************
* Function Name  : LCD_Clear
* Description    : ����Ļ����ָ������ɫ��������������� 0xffff
* Input          : - Color: Screen Color
* Output         : None
* Return         : None
* Attention		 : None
*******************************************************************************/
void LCD_Clear(uint16_t Color)
{
	uint32_t index;
	
	if( LCD_Code == HX8347D || LCD_Code == HX8347A )
	{
		LCD_WriteReg(0x02,0x00);                                                  
		LCD_WriteReg(0x03,0x00);  
		                
		LCD_WriteReg(0x04,0x00);                           
		LCD_WriteReg(0x05,0xEF);  
		                 
		LCD_WriteReg(0x06,0x00);                           
		LCD_WriteReg(0x07,0x00);    
		               
		LCD_WriteReg(0x08,0x01);                           
		LCD_WriteReg(0x09,0x3F);     
	}
	else
	{	
		LCD_SetCursor(0,0); 
	}	

	LCD_WriteIndex(0x0022);
	for( index = 0; index < MAX_X * MAX_Y; index++ )
	{
		LCD_WriteData(Color);
	}
}

/******************************************************************************
* Function Name  : LCD_BGR2RGB
* Description    : RRRRRGGGGGGBBBBB ��Ϊ BBBBBGGGGGGRRRRR ��ʽ
* Input          : - color: BRG ��ɫֵ  
* Output         : None
* Return         : RGB ��ɫֵ
* Attention		 : �ڲ���������
*******************************************************************************/
static uint16_t LCD_BGR2RGB(uint16_t color)
{
	uint16_t  r, g, b, rgb;
	
	b = ( color>>0 )  & 0x1f;
	g = ( color>>5 )  & 0x3f;
	r = ( color>>11 ) & 0x1f;
	
	rgb =  (b<<11) + (g<<5) + (r<<0);
	
	return( rgb );
}

/******************************************************************************
* Function Name  : LCD_GetPoint
* Description    : ��ȡָ���������ɫֵ
* Input          : - Xpos: Row Coordinate
*                  - Xpos: Line Coordinate 
* Output         : None
* Return         : Screen Color
* Attention		 : None
*******************************************************************************/
uint16_t LCD_GetPoint(uint16_t Xpos,uint16_t Ypos)
{
	uint16_t dummy;
	
	LCD_SetCursor(Xpos,Ypos);
	LCD_WriteIndex(0x0022);  
	
	switch( LCD_Code )
	{
		case ST7781:
		case LGDP4531:
		case LGDP4535:
		case SSD1289:
		case SSD1298:
             dummy = LCD_ReadData();   /* Empty read */
             dummy = LCD_ReadData(); 	
 		     return  dummy;	      
	    case HX8347A:
	    case HX8347D:
             {
		        uint8_t red,green,blue;
				
				dummy = LCD_ReadData();   /* Empty read */

		        red = LCD_ReadData() >> 3; 
                green = LCD_ReadData() >> 2; 
                blue = LCD_ReadData() >> 3; 
                dummy = (uint16_t) ( ( red<<11 ) | ( green << 5 ) | blue ); 
		     }	
	         return  dummy;

        default:	/* 0x9320 0x9325 0x9328 0x9331 0x5408 0x1505 0x0505 0x9919 */
             dummy = LCD_ReadData();   /* Empty read */
             dummy = LCD_ReadData(); 	
 		     return  LCD_BGR2RGB( dummy );
	}
}

/******************************************************************************
* Function Name  : LCD_SetPoint
* Description    : ��ָ�����껭��
* Input          : - Xpos: Row Coordinate
*                  - Ypos: Line Coordinate 
* Output         : None
* Return         : None
* Attention		 : None
*******************************************************************************/
void LCD_SetPoint(uint16_t Xpos,uint16_t Ypos,uint16_t point)
{
	if( Xpos >= MAX_X || Ypos >= MAX_Y )
	{
		return;
	}
	LCD_SetCursor(Xpos,Ypos);
	LCD_WriteReg(0x0022,point);
}

/******************************************************************************
* Function Name  : LCD_DrawLine
* Description    : Bresenham's line algorithm
* Input          : - x1: A��������
*                  - y1: A�������� 
*				   - x2: B��������
*				   - y2: B�������� 
*				   - color: ����ɫ
* Output         : None
* Return         : None
* Attention		 : None
*******************************************************************************/	 
void LCD_DrawLine( uint16_t x0, uint16_t y0, uint16_t x1, uint16_t y1 , uint16_t color )
{
    short dx,dy;    
    short temp;      

    if( x0 > x1 )    
    {
	    temp = x1;
		x1 = x0;
		x0 = temp;   
    }
    if( y0 > y1 )    
    {
		temp = y1;
		y1 = y0;
		y0 = temp;   
    }
  
	dx = x1-x0;     
	dy = y1-y0;    

    if( dx == 0 )    
    {
        do
        { 
            LCD_SetPoint(x0, y0, color);  
            y0++;
        }
        while( y1 >= y0 ); 
		return; 
    }
    if( dy == 0 )    
    {
        do
        {
            LCD_SetPoint(x0, y0, color);  
            x0++;
        }
        while( x1 >= x0 ); 
		return;
    }

    if( dx > dy )                        
    {
	    temp = 2 * dy - dx;              
        while( x0 != x1 )
        {
	        LCD_SetPoint(x0,y0,color);    
	        x0++;                         
	        if( temp > 0 )              
	        {
	            y0++;                     
	            temp += 2 * dy - 2 * dx; 
	 	    }
            else         
            {
			    temp += 2 * dy;          
			}       
        }
        LCD_SetPoint(x0,y0,color);
    }  
    else
    {
	    temp = 2 * dx - dy;                   
        while( y0 != y1 )
        {
	 	    LCD_SetPoint(x0,y0,color);     
            y0++;                 
            if( temp > 0 )           
            {
                x0++;               
                temp+=2*dy-2*dx; 
            }
            else
			{
                temp += 2 * dy;
			}
        } 
        LCD_SetPoint(x0,y0,color);
	}
} 


#if 0  /* === RIMOSSO: logica di gioco vecchia (ora in render.c / game.c) === */
void DrawGridFromVector(const uint8_t vector[24], uint16_t row, uint16_t color) {
    // Dimensioni dello schermo e della griglia
    uint16_t grid_cols = 24; // Numero di colonne
    uint16_t grid_rows = 32; // Numero di righe
    int square_width = 10;   // Larghezza di una cella
    int square_height = 10; // Altezza di una cella
	
    

    // Itera sulle colonne
		uint16_t col;

    for ( col = 0; col < grid_cols; col++) {
        // Coordinate del quadrato
            uint16_t square_x0 = col * square_width;          // Estremo sinistro
            uint16_t square_y0 = row * square_height;         // Estremo superiore
        // Se il valore del vettore per questa colonna � 1, disegna il quadrato
				if (vector[col] == 1) {
            uint16_t square_x1 = square_x0 + square_width - 1; // Estremo destro
            uint16_t square_y1 = square_y0 + square_height - 1; // Estremo inferiore 
					
				
            // Disegna il quadrato pieno
					uint16_t y;
					
					for(y = square_y0; y<=square_y0+square_height;y++){
						LCD_DrawLine(square_x0,y,square_x1,y,color);

					}

    }
				 else if(vector[col] == 0){
				 //PILLS
				
					if(food<240){ 
						if ((row<16 | row>22) | (col<10 | col>13)){	
									LCD_DrawFilledCircle(square_x0+5,square_y0+5 ,  1, White);
									food++;	
							}
						}
					}
			}
	}
/**/
/*disegna powerpills*/
void DrawpowerPills(const uint8_t vector[24], uint16_t row, uint16_t color) {
    // Dimensioni dello schermo e della griglia
    uint16_t grid_cols = 24; // Numero di colonne
    uint16_t grid_rows = 32; // Numero di righe
    int square_width = 10;   // Larghezza di una cella
    int square_height = 10; // Altezza di una cella
	
    

    // Itera sulle colonne
		uint16_t col;

    for ( col = 0; col < grid_cols; col++) {
        // Coordinate del quadrato
            uint16_t square_x0 = col * square_width;          // Estremo sinistro
            uint16_t square_y0 = row * square_height;         // Estremo superiore
       
			 if(vector[col] == 0){
				 //PILLS
				
	
		if((rand()%10 )==0 && powerPills<6){
					if ((row<16 | row>22) | (col<10 | col>13)){	//disegna powerpills
									LCD_DrawFilledCircle(square_x0+5,square_y0+5 ,  2, Red);
									powerPills++;
						
							}
			
							}
						}
					
			}
	}
	/*disegna powerpills*/
void SetPowerPills(){
	/* Spec 2: power pill in posizione e tempo casuali, su una pillola standard esistente */
	static int seeded=0;
	int attempts, r, c, cx, cy;
	if(!seeded){ srand(seed_ctr); seeded=1; }     /* seed casuale al primo utilizzo */
	if(powerPills>=6) return;                      /* massimo 6 power pill nella partita */
	if((rand()%POWERPILL_SPAWN_PROB)!=0) return;   /* tempo di comparsa casuale */
	for(attempts=0; attempts<25; attempts++){      /* posizione casuale */
		r = 4 + (rand()%26);                       /* righe 4..29 */
		c = rand()%24;                             /* colonne 0..23 */
		cx = c*10+5;
		cy = r*10+5;
		if(LCD_GetPoint(cx,cy)==0xFFFF){           /* solo dove c'e' una pillola standard */
			LCD_DrawFilledCircle(cx, cy, 2, Red);
			powerPills++;
			break;
		}
	}
	return;
/* (generazione power pill ora gestita all'inizio della funzione) */

	}
	
	
/*disegna cerchio*/
#endif /* === fine logica vecchia (DrawGridFromVector/DrawpowerPills/SetPowerPills) === */
void LCD_DrawFilledCircle(uint16_t centerX, uint16_t centerY, uint16_t radius, uint16_t color) {
    int16_t x = 0;
    int16_t y = radius;
    int16_t p = 1 - radius; // Decision parameter per Bresenham

    // Disegna le righe per riempire il cerchio
    while (x <= y) {
        // Disegna linee orizzontali per ogni coppia di punti
        LCD_DrawLine(centerX - x, centerY + y, centerX + x, centerY + y, color); // Segmento superiore
        LCD_DrawLine(centerX - x, centerY - y, centerX + x, centerY - y, color); // Segmento inferiore
        LCD_DrawLine(centerX - y, centerY + x, centerX + y, centerY + x, color); // Segmento laterale superiore
        LCD_DrawLine(centerX - y, centerY - x, centerX + y, centerY - x, color); // Segmento laterale inferiore

        // Aggiorna i punti secondo l'algoritmo del cerchio
        if (p < 0) {
            p += 2 * x + 3;
        } else {
            p += 2 * (x - y) + 5;
            y--;
        }
        x++;
    }
}


/*disegna cerchio*/


/*disegna griglia */
#if 0  /* === RIMOSSO: LCD_DrawLevel vecchio (ora render_level in render.c) === */
void LCD_DrawLevel(){

// Riga 4

DrawGridFromVector(linea4, 4, Blue);

// Riga 5

DrawGridFromVector(linea5, 5, Blue);

// Riga 6

DrawGridFromVector(linea6, 6, Blue);

// Riga 7

DrawGridFromVector(linea7, 7, Blue);

// Riga 8

DrawGridFromVector(linea8, 8, Blue);

// Riga 9

DrawGridFromVector(linea9, 9, Blue);

// Riga 10

DrawGridFromVector(linea10, 10, Blue);

// Riga 11

DrawGridFromVector(linea11, 11, Blue);

// Riga 12

DrawGridFromVector(linea12, 12, Blue);

// Riga 13

DrawGridFromVector(linea13, 13, Blue);

// Riga 14

DrawGridFromVector(linea14, 14, Blue);

// Riga 15

DrawGridFromVector(linea15, 15, Blue);

// Riga 16

DrawGridFromVector(linea16, 16, Blue);

// Riga 17

DrawGridFromVector(linea17, 17, Blue);

// Riga 18

DrawGridFromVector(linea18, 18, Blue);

// Riga 19

DrawGridFromVector(linea19, 19, Blue);

// Riga 20

DrawGridFromVector(linea20, 20, Blue);

// Riga 21

DrawGridFromVector(linea21, 21, Blue);

// Riga 22

DrawGridFromVector(linea22, 22, Blue);

// Riga 23

DrawGridFromVector(linea23, 23, Blue);

// Riga 24

DrawGridFromVector(linea24, 24, Blue);

// Riga 25

DrawGridFromVector(linea25, 25, Blue);

// Riga 26

DrawGridFromVector(linea26, 26, Blue);

// Riga 27

DrawGridFromVector(linea27, 27, Blue);

// Riga 28

DrawGridFromVector(linea28, 28, Blue);

// Riga 29

DrawGridFromVector(linea29, 29, Blue);

}

/*disegna griglia */




/******************************************************************************
* Function Name  : PutChar
* Description    : ��Lcd��������λ����ʾһ���ַ�
* Input          : - Xpos: ˮƽ���� 
*                  - Ypos: ��ֱ����  
*				   - ASCI: ��ʾ���ַ�
*				   - charColor: �ַ���ɫ   
*				   - bkColor: ������ɫ 
* Output         : None
* Return         : None
* Attention		 : None
*******************************************************************************/
#endif /* === fine LCD_DrawLevel vecchio === */
void PutChar( uint16_t Xpos, uint16_t Ypos, uint8_t ASCI, uint16_t charColor, uint16_t bkColor )
{
	uint16_t i, j;
    uint8_t buffer[16], tmp_char;
    GetASCIICode(buffer,ASCI);  /* ȡ��ģ���� */
    for( i=0; i<16; i++ )
    {
        tmp_char = buffer[i];
        for( j=0; j<8; j++ )
        {
            if( ((tmp_char >> (7 - j)) & 0x01) == 0x01 )
            {
                LCD_SetPoint( Xpos + j, Ypos + i, charColor );  /* �ַ���ɫ */
            }
            else
            {
                LCD_SetPoint( Xpos + j, Ypos + i, bkColor );  /* ������ɫ */
            }
        }
    }
}

/******************************************************************************
* Function Name  : GUI_Text
* Description    : ��ָ��������ʾ�ַ���
* Input          : - Xpos: ������
*                  - Ypos: ������ 
*				   - str: �ַ���
*				   - charColor: �ַ���ɫ   
*				   - bkColor: ������ɫ 
* Output         : None
* Return         : None
* Attention		 : None
*******************************************************************************/
void GUI_Text(uint16_t Xpos, uint16_t Ypos, uint8_t *str,uint16_t Color, uint16_t bkColor)
{
    uint8_t TempChar;
    do
    {
        TempChar = *str++;  
        PutChar( Xpos, Ypos, TempChar, Color, bkColor );    
        if( Xpos < MAX_X - 8 )
        {
            Xpos += 8;
        } 
        else if ( Ypos < MAX_Y - 16 )
        {
            Xpos = 0;
            Ypos += 16;
        }   
        else
        {
            Xpos = 0;
            Ypos = 0;
        }    
    }
    while ( *str != 0 );
}


#if 0  /* === RIMOSSO: vecchie LCD_DrawPacMan/aggiornapunteggio/movimento/draw_first_life/erasePause === */
void LCD_DrawPacMan(uint16_t centerX, uint16_t centerY,uint16_t  color) {
		int16_t x = 0;
    int16_t y = 4;
    int16_t p = 1 - 4; // Decision parameter per Bresenham

    // Disegna le righe per riempire il cerchio
    while (x <= y) {
        // Disegna linee orizzontali per ogni coppia di punti
        LCD_DrawLine(centerX - x, centerY + y, centerX + x, centerY + y, color); // Segmento superiore
        LCD_DrawLine(centerX - x, centerY - y, centerX + x, centerY - y, color); // Segmento inferiore
        LCD_DrawLine(centerX - y, centerY + x, centerX + y, centerY + x, color); // Segmento laterale superiore
        LCD_DrawLine(centerX - y, centerY - x, centerX + y, centerY - x, color); // Segmento laterale inferiore

        // Aggiorna i punti secondo l'algoritmo del cerchio
        if (p < 0) {
            p += 2 * x + 3;
        } else {
            p += 2 * (x - y) + 5;
            y--;
        }
        x++;
    }
}




void aggiornapunteggio()
	{
				if(game_state!=0) return;        /* partita finita: stop aggiornamenti */
				sprintf(scoreText,"%d", Points);
				GUI_Text(190, 20, (uint8_t *) scoreText, Black, White);
				
		if(extralifecount>=1000){            /* +1 vita ogni 1000 punti (nessun cap) */
												extralifecount=0;
												life=life+1;						
												LCD_DrawPacMan(x_life,y_life,yellow);
												x_life=x_life+20;
															}
				//check vittoria
				if (food==0){
				flag_movimento=0;
				disable_timer(0);
				disable_timer(1);
				disable_timer(3);
				game_state=1;            /* Vittoria: schermata mostrata nel main */
				}
																	
															
}

/*movimento*/
void movimento(){
	
			switch (movement){
				
					case 0://select
						break;
					case 1:// up
							if(first_time==0){//timer tempo di gioco
									enable_timer(0);
										first_time++;
									}
								if(LCD_GetPoint( pacMan_position[0],(pacMan_position[1]-10))!=0x0000F800){//se non trovi ostacolo vai avanti
									
									x=pacMan_position[0];
									y=pacMan_position[1]-10;
									LCD_DrawPacMan(pacMan_position[0],pacMan_position[1],Black);
									
									if(LCD_GetPoint( x,y)==0x0000FFFF){
											Points=Points+10;		
											extralifecount=extralifecount+10;
										  food--;
										}
									
									else if(LCD_GetPoint( x,y)==0x0000001F){
											Points=Points+50;		
											extralifecount=extralifecount+50;
										food--;
										}	
										
										
									LCD_DrawPacMan(x,y,yellow);//vai sopra
									pacMan_position[0]=x;
									pacMan_position[1]=y;
										
									
							}
						break;
					case 2:// down
							if(first_time==0){//timer tempo di gioco
									enable_timer(0);
										first_time++;
									}
									if(LCD_GetPoint( pacMan_position[0],(pacMan_position[1]+10))!=0x0000F800){
								x=pacMan_position[0];
								y=pacMan_position[1]+10;
								LCD_DrawPacMan(pacMan_position[0],pacMan_position[1],Black);
									if(LCD_GetPoint( x,y)==0x0000FFFF){
													Points=Points+10;
											extralifecount=extralifecount+10;
										food--;
										}		
									else if(LCD_GetPoint( x,y)==0x0000001F){
											Points=Points+50;		
												extralifecount=extralifecount+50;
												food--;
										}	
								LCD_DrawPacMan(x,y,yellow);//vai sotto
								pacMan_position[0]=x;
								pacMan_position[1]=y;
						}
						
						break;
					case 3: //left
							if(first_time==0){//timer tempo di gioco
									enable_timer(0);
										first_time++;
									}
						if((pacMan_position[0]==5)&&(pacMan_position[1]==195)){//teleport sx-dx
						
								if(LCD_GetPoint(230,195)==0x0000FFFF){//
								 food--;
								}
						LCD_DrawPacMan(pacMan_position[0],pacMan_position[1],Black);
						LCD_DrawPacMan(235,195,yellow);
						pacMan_position[0]=235;
						pacMan_position[1]=195;
						break;
					}
					
					if(LCD_GetPoint( (pacMan_position[0]-10),pacMan_position[1])!=0x0000F800){
						LCD_DrawPacMan(pacMan_position[0],pacMan_position[1],Black);
						x=pacMan_position[0]-10;
					  y=pacMan_position[1];
						
						if(LCD_GetPoint( x,y)==0x0000FFFF){//pills
												Points=Points+10;
								extralifecount=extralifecount+10;
									food--;
										}
						else if(LCD_GetPoint( x,y)==0x0000001F){//powerpills
											Points=Points+50;		
											extralifecount=extralifecount+50;
											food--;
										}	
						LCD_DrawPacMan(x,y,yellow);
						pacMan_position[0]=x;
						pacMan_position[1]=y;
						
					}

						break;
					case 4: // right
										if(first_time==0){//timer tempo di gioco
									enable_timer(0);
										first_time++;
									}
								if((pacMan_position[0]==235)&&(pacMan_position[1]==195)){//teleport dx-sx
						if(LCD_GetPoint(0,195)==0x0000FFFF){//
								 food--;
								}			
						LCD_DrawPacMan(pacMan_position[0],pacMan_position[1],Black);
						LCD_DrawPacMan(5,195,yellow);
							pacMan_position[0]=5;
							pacMan_position[1]=195;
							break;
						}

					if(LCD_GetPoint( (pacMan_position[0]+10),pacMan_position[1])!=0x0000F800){

							LCD_DrawPacMan(pacMan_position[0],pacMan_position[1],Black);
							x=pacMan_position[0]+10;
							y=pacMan_position[1];
						
								if(LCD_GetPoint( x,y)==0x0000FFFF){
												Points=Points+10;
												extralifecount=extralifecount+10;
												food--;
										}
								else if(LCD_GetPoint( x,y)==0x0000001F){
											Points=Points+50;		
											extralifecount=extralifecount+50;
											food--;
										
										}	
							LCD_DrawPacMan(x,y,yellow);
							pacMan_position[0]=x;
							pacMan_position[1]=y;
					
					}
						break;						
					
				}
			}



/*movimento*/
			

	
			
/* Disegna l'icona della vita iniziale (Spec 6) */
void draw_first_life(void){
	LCD_DrawPacMan(x_life, y_life, yellow);
	x_life = x_life + 20;
}

/* Cancella il messaggio "PAUSE" ridisegnando la central box (cols 10-13, righe 16-22).
   Quella zona non contiene pillole, quindi le pillole non vengono toccate/perse. */
void erasePause(void){
	uint8_t* boxrows[7] = {linea16,linea17,linea18,linea19,linea20,linea21,linea22};
	int r, c, yy, x0, y0;
	uint16_t col;
	for(r=0; r<7; r++){
		for(c=10; c<=13; c++){
			col = (boxrows[r][c]==1) ? Blue : Black;
			x0 = c*10;
			y0 = (r+16)*10;
			for(yy=y0; yy<=y0+10; yy++){
				LCD_DrawLine(x0, yy, x0+9, yy, col);
			}
		}
	}
}

#endif /* === fine logica di gioco vecchia === */

/*********************************************************************************************************
      END FILE
*********************************************************************************************************/
