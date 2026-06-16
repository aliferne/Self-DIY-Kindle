#include "LCD.h"
#include <stdint.h>
#include <string.h>
#include <stdlib.h>
#include "FONT.h"
#include "disp_drv.h"
#include "bsp_spi.h"
#include "bsp_gpio.h"
#include "bsp_handle.h"


static void (*lcd_delay_ms)(uint32_t ms) = NULL;
static Disp_Src_t *lcd_src = NULL;

void lcd_assign_src(Disp_Src_t *src)
{
    lcd_src = src;
}

void lcd_assign_delay(void (*cb)(uint32_t ms))
{
    lcd_delay_ms = cb;
}

//LCD??????????????	   
uint16_t POINT_COLOR = 0x0000;	//???????
uint16_t BACK_COLOR = 0xFFFF;  //????? 

//????LCD???????
//????????
_lcd_dev lcddev;
		 					    
//§Õ?????????
//data:??????
static void LCD_WR_REG(uint8_t cmd)
{
    ASSERT_FAIL(lcd_src == NULL || lcd_src->dc_pin == NULL, return);
    gpio_write(lcd_src->dc_pin, GPIO_Level_Low);
    spi_cs_select(lcd_src->spi);
    spi_write(lcd_src->spi, &cmd, 1);
    spi_cs_deselect(lcd_src->spi);
}

//§Õ???????
//data:??????
static void LCD_WR_DATA(uint8_t data)
{
    ASSERT_FAIL(lcd_src == NULL || lcd_src->dc_pin == NULL, return);
    gpio_write(lcd_src->dc_pin, GPIO_Level_High);
    spi_cs_select(lcd_src->spi);
    spi_write(lcd_src->spi, &data, 1);
    spi_cs_deselect(lcd_src->spi);
}

//LCD§ÕGRAM
//RGB_Code:????
static void LCD_WriteRAM(uint16_t RGB_Code)
{
    ASSERT_FAIL(lcd_src == NULL || lcd_src->dc_pin == NULL, return);
    uint8_t data[2] = {(uint8_t)(RGB_Code >> 8), (uint8_t)(RGB_Code & 0xFF)};
    gpio_write(lcd_src->dc_pin, GPIO_Level_High);
    spi_cs_select(lcd_src->spi);
    spi_write(lcd_src->spi, data, 2);
    spi_cs_deselect(lcd_src->spi);
}

//§Õ?????
//LCD_Reg:????????
//LCD_RegValue:?§Õ????
static void LCD_WriteReg(uint16_t LCD_Reg, uint16_t LCD_RegValue)
{	
    LCD_WR_REG((uint8_t)LCD_Reg);  
    LCD_WriteRAM(LCD_RegValue);	    		 
}   

//???§ÕGRAM
static void LCD_WriteRAM_Prepare(void)
{
    LCD_WR_REG((uint8_t)lcddev.wramcmd);
} 

//LCD???????
void LCD_DisplayOn(void)
{					   
    LCD_WR_REG(0X29);	//???????
}	 
//LCD??????
void LCD_DisplayOff(void)
{	   
    LCD_WR_REG(0X28);	//??????
}   
//???¨´??¦Ë??
//Xpos:??????
//Ypos:??????
void LCD_SetCursor(uint16_t Xpos, uint16_t Ypos)
{	 	    
    LCD_WR_REG((uint8_t)lcddev.setxcmd); 
    LCD_WR_DATA((uint8_t)(Xpos >> 8));
    LCD_WR_DATA((uint8_t)(Xpos & 0XFF)); 			 
    LCD_WR_REG((uint8_t)lcddev.setycmd); 
    LCD_WR_DATA((uint8_t)(Ypos >> 8));
    LCD_WR_DATA((uint8_t)(Ypos & 0XFF)); 		 
} 		 
   
//????
//x,y:????
//POINT_COLOR:???????
void LCD_DrawPoint(uint16_t x,uint16_t y)
{
    LCD_SetCursor(x,y);		//???¨´??¦Ë?? 
    LCD_WriteRAM_Prepare();	//???§Õ??GRAM
    LCD_WriteRAM(POINT_COLOR); 
}	 
//???????
//x,y:????
//color:???
void LCD_Fast_DrawPoint(uint16_t x,uint16_t y,uint16_t color)
{	   
    //???¨´??¦Ë??
    LCD_SetCursor(x,y); 	 
    //§Õ?????
    LCD_WriteReg(lcddev.wramcmd,color);
}


//dir:??????? 	0-0???????1-180???????2-270???????3-90?????
void LCD_Display_Dir(uint8_t dir)
{
    if(dir==0||dir==1)			//????
    {
        lcddev.dir=0;	//????
        lcddev.width=320;
        lcddev.height=480;

        lcddev.wramcmd=0X2C;
        lcddev.setxcmd=0X2A;
        lcddev.setycmd=0X2B;
        
        if(dir==0)        //0-0?????
        {
            LCD_WR_REG(0x36); 
            LCD_WR_DATA((1<<3)|(0<<7)|(1<<6)|(0<<5));
        }else							//1-180?????
        {
            LCD_WR_REG(0x36); 
            LCD_WR_DATA((1<<3)|(1<<7)|(0<<6)|(0<<5));		
        }
        
    }else if(dir==2||dir==3)
    {
        
        lcddev.dir=1;	//????
        lcddev.width=480;
        lcddev.height=320;

        lcddev.wramcmd=0X2C;
        lcddev.setxcmd=0X2A;
        lcddev.setycmd=0X2B; 

        if(dir==2)				//2-270?????
        {
            LCD_WR_REG(0x36); 
            LCD_WR_DATA((1<<3)|(1<<7)|(1<<6)|(1<<5));

        }else							//3-90?????
        {
            LCD_WR_REG(0x36); 
            LCD_WR_DATA((1<<3)|(0<<7)|(0<<6)|(1<<5));
        }		
    }	

    //???????????	
    LCD_WR_REG((uint8_t)lcddev.setxcmd); 
    LCD_WR_DATA(0);
    LCD_WR_DATA(0);
    LCD_WR_DATA((uint8_t)((lcddev.width-1)>>8));
    LCD_WR_DATA((uint8_t)((lcddev.width-1)&0XFF));
    LCD_WR_REG((uint8_t)lcddev.setycmd); 
    LCD_WR_DATA(0);
    LCD_WR_DATA(0);
    LCD_WR_DATA((uint8_t)((lcddev.height-1)>>8));
    LCD_WR_DATA((uint8_t)((lcddev.height-1)&0XFF));  
}	 
//???????,?????????????????????????(sx,sy).
//sx,sy:???????????(?????)
//width,height:??????????,???????0!!
//?????§³:width*height. 
void LCD_Set_Window(uint16_t sx,uint16_t sy,uint16_t width,uint16_t height)
{    
    uint16_t twidth,theight;
    twidth=sx+width-1;
    theight=sy+height-1;

    LCD_WR_REG((uint8_t)lcddev.setxcmd); 
    LCD_WR_DATA((uint8_t)(sx>>8)); 
    LCD_WR_DATA((uint8_t)(sx&0XFF));	 
    LCD_WR_DATA((uint8_t)(twidth>>8)); 
    LCD_WR_DATA((uint8_t)(twidth&0XFF));  
    LCD_WR_REG((uint8_t)lcddev.setycmd); 
    LCD_WR_DATA((uint8_t)(sy>>8)); 
    LCD_WR_DATA((uint8_t)(sy&0XFF)); 
    LCD_WR_DATA((uint8_t)(theight>>8)); 
    LCD_WR_DATA((uint8_t)(theight&0XFF)); 
}
//?????lcd
void LCD_Init(void)
{
    ASSERT_FAIL(lcd_src == NULL, return);
    ASSERT_FAIL(lcd_src->rst_pin == NULL, return);

    gpio_write(lcd_src->rst_pin, GPIO_Level_High);
    if(lcd_delay_ms) lcd_delay_ms(1);
    gpio_write(lcd_src->rst_pin, GPIO_Level_Low);
    if(lcd_delay_ms) lcd_delay_ms(10);
    gpio_write(lcd_src->rst_pin, GPIO_Level_High);
    if(lcd_delay_ms) lcd_delay_ms(120);

//************* Start Initial Sequence **********//
    if(lcd_delay_ms) lcd_delay_ms(120); // Delay 120ms
    LCD_WR_REG(0x11); // Sleep Out
    if(lcd_delay_ms) lcd_delay_ms(120); // Delay 120ms
    LCD_WR_REG(0xf0) ;
    LCD_WR_DATA(0xc3) ;
    LCD_WR_REG(0xf0) ;
    LCD_WR_DATA(0x96) ;
    LCD_WR_REG(0x36);
    LCD_WR_DATA(0x48); 
    LCD_WR_REG(0x3A);
    LCD_WR_DATA(0x55);
    LCD_WR_REG(0xB4);
    LCD_WR_DATA(0x01);
    LCD_WR_REG(0xB7) ;
    LCD_WR_DATA(0xC6) ;
    LCD_WR_REG(0xe8);
    LCD_WR_DATA(0x40);
    LCD_WR_DATA(0x8a);
    LCD_WR_DATA(0x00);
    LCD_WR_DATA(0x00);
    LCD_WR_DATA(0x29);
    LCD_WR_DATA(0x19);
    LCD_WR_DATA(0xa5);
    LCD_WR_DATA(0x33);
    LCD_WR_REG(0xc1);
    LCD_WR_DATA(0x06);
    LCD_WR_REG(0xc2);
    LCD_WR_DATA(0xa7);
    LCD_WR_REG(0xc5);
    LCD_WR_DATA(0x18);
    LCD_WR_REG(0xe0); //Positive Voltage Gamma Control
    LCD_WR_DATA(0xf0);
    LCD_WR_DATA(0x09);
    LCD_WR_DATA(0x0b);
    LCD_WR_DATA(0x06);
    LCD_WR_DATA(0x04);
    LCD_WR_DATA(0x15);
    LCD_WR_DATA(0x2f);
    LCD_WR_DATA(0x54);
    LCD_WR_DATA(0x42);
    LCD_WR_DATA(0x3c);
    LCD_WR_DATA(0x17);
    LCD_WR_DATA(0x14);
    LCD_WR_DATA(0x18);
    LCD_WR_DATA(0x1b);
    LCD_WR_REG(0xe1); //Negative Voltage Gamma Control
    LCD_WR_DATA(0xf0);
    LCD_WR_DATA(0x09);
    LCD_WR_DATA(0x0b);
    LCD_WR_DATA(0x06);
    LCD_WR_DATA(0x04);
    LCD_WR_DATA(0x03);
    LCD_WR_DATA(0x2d);
    LCD_WR_DATA(0x43);
    LCD_WR_DATA(0x42);
    LCD_WR_DATA(0x3b);
    LCD_WR_DATA(0x16);
    LCD_WR_DATA(0x14);
    LCD_WR_DATA(0x17);
    LCD_WR_DATA(0x1b);
    LCD_WR_REG(0xf0);
    LCD_WR_DATA(0x3c);
    LCD_WR_REG(0xf0);
    LCD_WR_DATA(0x69);
    LCD_WR_REG(0x21);
    if(lcd_delay_ms) lcd_delay_ms(120); //Delay 120ms
    LCD_WR_REG(0x29);// Display on
} 



//?????????????	 
//x,y:????
//?????:???????
uint16_t LCD_ReadPoint(uint16_t x,uint16_t y)
{
    uint8_t reg = 0x2E;
    uint16_t color;
    if (x >= lcddev.width || y >= lcddev.height) return 0;
    LCD_SetCursor(x, y);
    ASSERT_FAIL(lcd_src == NULL || lcd_src->dc_pin == NULL, return 0);

    /* ????????? 0x2E */
    gpio_write(lcd_src->dc_pin, GPIO_Level_Low);
    spi_cs_select(lcd_src->spi);
    spi_write(lcd_src->spi, &reg, 1);

    /* ???? 2 ????????????spi ?????ÛF?? dummy ????????? */
    gpio_write(lcd_src->dc_pin, GPIO_Level_High);
    {
        uint8_t rx[2];
        uint8_t dummy[2] = {0};
        spi_write_read(lcd_src->spi, dummy, rx, 2);
        color = ((uint16_t)rx[0] << 8) | rx[1];
    }
    spi_cs_deselect(lcd_src->spi);

    return color;
}		
  
//????????
//color:???????????
void LCD_Clear(uint16_t color)
{
    uint32_t index=0;      
    uint32_t totalpoint=lcddev.width;
    totalpoint*=lcddev.height; 			//????????
    LCD_SetCursor(0x00,0x0000);	//???¨´??¦Ë?? 
    LCD_WriteRAM_Prepare();     		//???§Õ??GRAM	  	  
    for(index=0;index<totalpoint;index++)LCD_WriteRAM(color);
}  
//????????????????????
//?????§³:(xend-xsta+1)*(yend-ysta+1)
//xsta
//color:????????
void LCD_Fill(uint16_t sx,uint16_t sy,uint16_t ex,uint16_t ey,uint16_t color)
{          
    uint16_t i,j;
    uint16_t xlen=0;
    uint16_t temp;
    if((lcddev.id==0X6804)&&(lcddev.dir==1))	//6804???????????????  
    {
        temp=sx;
        sx=sy;
        sy=lcddev.width-ex-1;	  
        ex=ey;
        ey=lcddev.width-temp-1;
        lcddev.dir=0;	 
        lcddev.setxcmd=0X2A;
        lcddev.setycmd=0X2B;  	 			
        LCD_Fill(sx,sy,ex,ey,color);  
        lcddev.dir=1;	 
        lcddev.setxcmd=0X2B;
        lcddev.setycmd=0X2A;  	 
    }else
    {
        xlen=ex-sx+1;	 
        for(i=sy;i<=ey;i++)
        {
            LCD_SetCursor(sx,i);      				//???¨´??¦Ë?? 
            LCD_WriteRAM_Prepare();     			//???§Õ??GRAM	  
            for(j=0;j<xlen;j++)LCD_WriteRAM(color);	//???¨´??¦Ë?? 	    
        }
    }
}  
//??????????????????????			 
//(sx,sy),(ex,ey):?????¦Æ??????,?????§³?:(ex-sx+1)*(ey-sy+1)   
//color:????????
void LCD_Color_Fill(uint16_t sx,uint16_t sy,uint16_t ex,uint16_t ey,uint16_t *color)
{  
    uint16_t height,width;
    uint16_t i,j;
    width=ex-sx+1; 			//??????????
    height=ey-sy+1;			//???
    for(i=0;i<height;i++)
    {
        LCD_SetCursor(sx,sy+i);   	//???¨´??¦Ë?? 
        LCD_WriteRAM_Prepare();     //???§Õ??GRAM
        for(j=0;j<width;j++)LCD_WriteRAM(color[i*width+j]);//§Õ?????? 
    }  
}
//????
//x1,y1:???????
//x2,y2:???????  
void LCD_DrawLine(uint16_t x1, uint16_t y1, uint16_t x2, uint16_t y2)
{
    uint16_t t; 
    int xerr=0,yerr=0,delta_x,delta_y,distance; 
    int incx,incy,uRow,uCol; 
    delta_x=x2-x1; //???????????? 
    delta_y=y2-y1; 
    uRow=x1; 
    uCol=y1; 
    if(delta_x>0)incx=1; //??????????? 
    else if(delta_x==0)incx=0;//????? 
    else {incx=-1;delta_x=-delta_x;} 
    if(delta_y>0)incy=1; 
    else if(delta_y==0)incy=0;//???? 
    else{incy=-1;delta_y=-delta_y;} 
    if( delta_x>delta_y)distance=delta_x; //???????????????? 
    else distance=delta_y; 
    for(t=0;t<=distance+1;t++ )//??????? 
    {  
        LCD_DrawPoint(uRow,uCol);//???? 
        xerr+=delta_x ; 
        yerr+=delta_y ; 
        if(xerr>distance) 
        { 
            xerr-=distance; 
            uRow+=incx; 
        } 
        if(yerr>distance) 
        { 
            yerr-=distance; 
            uCol+=incy; 
        } 
    }  
}    
//??????	  
//(x1,y1),(x2,y2):???¦Å???????
void LCD_DrawRectangle(uint16_t x1, uint16_t y1, uint16_t x2, uint16_t y2)
{
    LCD_DrawLine(x1,y1,x2,y1);
    LCD_DrawLine(x1,y1,x1,y2);
    LCD_DrawLine(x1,y2,x2,y2);
    LCD_DrawLine(x2,y1,x2,y2);
}
//?????¦Ë???????????§³???
//(x,y):?????
//r    :??
void LCD_Draw_Circle(uint16_t x0,uint16_t y0,uint8_t r)
{
    int a,b;
    int di;
    a=0;b=r;	  
    di=3-(r<<1);             //?§Ø??????¦Ë?????
    while(a<=b)
    {
        LCD_DrawPoint(x0+a,y0-b);             //5
        LCD_DrawPoint(x0+b,y0-a);             //0           
        LCD_DrawPoint(x0+b,y0+a);             //4               
        LCD_DrawPoint(x0+a,y0+b);             //6 
        LCD_DrawPoint(x0-a,y0+b);             //1       
        LCD_DrawPoint(x0-b,y0+a);             
        LCD_DrawPoint(x0-a,y0-b);             //2             
        LCD_DrawPoint(x0-b,y0-a);             //7     	         
        a++;
        //???Bresenham?????     
        if(di<0)di +=4*a+6;	  
        else
        {
            di+=10+4*(a-b);   
            b--;
        } 						    
    }
} 									  
//?????¦Ë???????????
//x,y:???????
//num:?????????:" "--->"~"
//size:?????§³ 12/16/24
//mode:??????(1)??????????(0)
void LCD_ShowChar(uint16_t x,uint16_t y,uint8_t num,uint8_t size,uint8_t mode)
{  							  
    uint8_t temp,t1,t;
    uint16_t y0=y;
    uint8_t csize=(size/8+((size%8)?1:0))*(size/2);		//??????????????????????????????	
    num=num-' ';//???????????ASCII?????????????????-' '??????????????
    for(t=0;t<csize;t++)
    {   
        if(size==12)temp=asc2_1206[num][t]; 	 	//????1206????
        else if(size==16)temp=asc2_1608[num][t];	//????1608????
        else if(size==24)temp=asc2_2412[num][t];	//????2412????
        else return;								//??§Ö????
        for(t1=0;t1<8;t1++)
        {			    
            if(temp&0x80)LCD_Fast_DrawPoint(x,y,POINT_COLOR);
            else if(mode==0)LCD_Fast_DrawPoint(x,y,BACK_COLOR);
            temp<<=1;
            y++;
            if(y>=lcddev.height)return;		//????????
            if((y-y0)==size)
            {
                y=y0;
                x++;
                if(x>=lcddev.width)return;	//????????
                break;
            }
        }  	 
    }  	    	   	 	  
}   
//m^n????
//?????:m^n?¦Ç?.
uint32_t LCD_Pow(uint8_t m,uint8_t n)
{
    uint32_t result=1;	 
    while(n--)result*=m;    
    return result;
}			 
//???????,??¦Ë?0,?????
//x,y :???????	 
//len :?????¦Ë??
//size:?????§³
//color:??? 
//num:???(0~4294967295);	 
void LCD_ShowNum(uint16_t x,uint16_t y,uint32_t num,uint8_t len,uint8_t size)
{         	
    uint8_t t,temp;
    uint8_t enshow=0;						   
    for(t=0;t<len;t++)
    {
        temp=(num/LCD_Pow(10,len-t-1))%10;
        if(enshow==0&&t<(len-1))
        {
            if(temp==0)
            {
                LCD_ShowChar(x+(size/2)*t,y,' ',size,0);
                continue;
            }else enshow=1; 
             
        }
        LCD_ShowChar(x+(size/2)*t,y,temp+'0',size,0); 
    }
} 
//???????,??¦Ë?0,???????
//x,y:???????
//num:???(0~999999999);	 
//len:????(????????¦Ë??)
//size:?????§³
//mode:
//[7]:0,?????;1,???0.
//[6:1]:????
//[0]:0,????????;1,???????.
void LCD_ShowxNum(uint16_t x,uint16_t y,uint32_t num,uint8_t len,uint8_t size,uint8_t mode)
{  
    uint8_t t,temp;
    uint8_t enshow=0;						   
    for(t=0;t<len;t++)
    {
        temp=(num/LCD_Pow(10,len-t-1))%10;
        if(enshow==0&&t<(len-1))
        {
            if(temp==0)
            {
                if(mode&0X80)LCD_ShowChar(x+(size/2)*t,y,'0',size,mode&0X01);  
                else LCD_ShowChar(x+(size/2)*t,y,' ',size,mode&0X01);  
                continue;
            }else enshow=1; 
             
        }
        LCD_ShowChar(x+(size/2)*t,y,temp+'0',size,mode&0X01); 
    }
} 
//????????
//x,y:???????
//width,height:?????§³  
//size:?????§³
//*p:???????????		  
void LCD_ShowString(uint16_t x,uint16_t y,uint16_t width,uint16_t height,uint8_t size,uint8_t *p)
{
    uint8_t x0=x;
    width+=x;
    height+=y;
    while((*p<='~')&&(*p>=' '))//?§Ø???????????!
    {       
        if(x>=width){x=x0;y+=size;}
        if(y>=height)break;//???
        LCD_ShowChar(x,y,*p,size,0);
        x+=size/2;
        p++;
    }
}


//???? 16*16
void GUI_DrawFont16(uint16_t x, uint16_t y, uint8_t *s, uint8_t mode)
{
    uint8_t i,j;
    uint16_t k;
    uint16_t HZnum;
    uint16_t x0=x;
    HZnum=sizeof(tfont16)/sizeof(typFNT_GB16);	//????????????
    
            
    for (k=0;k<HZnum;k++) 
    {
        if ((tfont16[k].Index[0]==*(s))&&(tfont16[k].Index[1]==*(s+1)))
        { 	LCD_Set_Window(x,y,16,16);
            LCD_WriteRAM_Prepare();
            for(i=0;i<16*2;i++)
            {
                for(j=0;j<8;j++)
                {	
                    if(!mode) //???????
                    {
                        if(tfont16[k].Msk[i]&(0x80>>j))	LCD_WriteRAM(POINT_COLOR);
                        else LCD_WriteRAM(BACK_COLOR);
                    }
                    else
                    {
                        //POINT_COLOR=fc;
                        if(tfont16[k].Msk[i]&(0x80>>j))	LCD_DrawPoint(x,y);//???????
                        x++;
                        if((x-x0)==16)
                        {
                            x=x0;
                            y++;
                            break;
                        }
                    }

                }
                
            }
            
            
        }				  	
        continue;  //??????????????????????????????????????????????
    }

    LCD_Set_Window(0,0,lcddev.width,lcddev.height);//???????????  
} 

//???? 24*24
void GUI_DrawFont24(uint16_t x, uint16_t y, uint8_t *s, uint8_t mode)
{
    uint8_t i,j;
    uint16_t k;
    uint16_t HZnum;
    uint16_t x0=x;
    HZnum=sizeof(tfont24)/sizeof(typFNT_GB24);	//????????????
        
    for (k=0;k<HZnum;k++) 
    {
        if ((tfont24[k].Index[0]==*(s))&&(tfont24[k].Index[1]==*(s+1)))
        { 	LCD_Set_Window(x,y,24,24);
            LCD_WriteRAM_Prepare();
            for(i=0;i<24*3;i++)
            {
                for(j=0;j<8;j++)
                {
                    if(!mode) //???????
                    {
                        if(tfont24[k].Msk[i]&(0x80>>j))	LCD_WriteRAM(POINT_COLOR);
                        else LCD_WriteRAM(BACK_COLOR);
                    }
                else
                {
                    //POINT_COLOR=fc;
                    if(tfont24[k].Msk[i]&(0x80>>j))	LCD_DrawPoint(x,y);//???????
                    x++;
                    if((x-x0)==24)
                    {
                        x=x0;
                        y++;
                        break;
                    }
                }
                }
            }
            
            
        }				  	
        continue;  //??????????????????????????????????????????????
    }

    LCD_Set_Window(0,0,lcddev.width,lcddev.height);//???????????  
}

//???? 32*32
void GUI_DrawFont32(uint16_t x, uint16_t y, uint8_t *s, uint8_t mode)
{
    uint8_t i,j;
    uint16_t k;
    uint16_t HZnum;
    uint16_t x0=x;
    HZnum=sizeof(tfont32)/sizeof(typFNT_GB32);	//????????????
    for (k=0;k<HZnum;k++) 
    {
        if ((tfont32[k].Index[0]==*(s))&&(tfont32[k].Index[1]==*(s+1)))
        { 	LCD_Set_Window(x,y,32,32);
            LCD_WriteRAM_Prepare();
            for(i=0;i<32*4;i++)
            {
                for(j=0;j<8;j++)
                {
                    if(!mode) //???????
                    {
                        if(tfont32[k].Msk[i]&(0x80>>j))	LCD_WriteRAM(POINT_COLOR);
                        else LCD_WriteRAM(BACK_COLOR);
                    }
                    else
                    {
                        //POINT_COLOR=fc;
                        if(tfont32[k].Msk[i]&(0x80>>j))	LCD_DrawPoint(x,y);//???????
                        x++;
                        if((x-x0)==32)
                        {
                            x=x0;
                            y++;
                            break;
                        }
                    }
                }
            }
            
            
        }				  	
        continue;  //??????????????????????????????????????????????
    }
    
    LCD_Set_Window(0,0,lcddev.width,lcddev.height);//???????????  
} 



//???????????????
void Show_Str(uint16_t x, uint16_t y, uint8_t *str, uint8_t size, uint8_t mode)
{			
    uint16_t x0=x;							  	  
    uint8_t bHz=0;     //??????????? 
    while(*str!=0)//????¦Ä????
    { 
        if(!bHz)
        {
            if(x>(lcddev.width-size/2)||y>(lcddev.height-size)) 
            return; 
            if(*str>0x80)bHz=1;//???? 
            else              //???
            {          
                if(*str==0x0D)//???§Ù???
                {         
                    y+=size;
                    x=x0;
                    str++; 
                }  
                else
                {
                    if(size>=24)//???????§Þ???12X24 16X32?????????,??8X16????
                    { 						
                    LCD_ShowChar(x,y,*str,24,mode);
                    x+=12; //???,???????? 
                    }
                    else
                    {
                    LCD_ShowChar(x,y,*str,size,mode);
                    x+=size/2; //???,???????? 
                    }
                } 
                str++; 
                
            }
        }else//???? 
        {   
            if(x>(lcddev.width-size)||y>(lcddev.height-size)) 
            return;  
            bHz=0;//?§Ü????    
            if(size==32)
            GUI_DrawFont32(x,y,str,mode);	 	
            else if(size==24)
            GUI_DrawFont24(x,y,str,mode);	
            else
            GUI_DrawFont16(x,y,str,mode);
                
            str+=2; 
            x+=size;//????????????	    
        }						 
    }   
}


//???40*40??
void Gui_Drawbmp16(uint16_t x,uint16_t y,const unsigned char *p) //???40*40??
{
    int i; 
    unsigned char picH,picL; 
    LCD_Set_Window(x,y,40,40);
    LCD_WriteRAM_Prepare();	
    
    for(i=0;i<40*40;i++)
    {	
        picL=*(p+i*2);	//?????¦Ë???
        picH=*(p+i*2+1);				
        LCD_WriteRAM(picH<<8|picL);  						
    }	
    LCD_Set_Window(0,0,lcddev.width,lcddev.height);//??????????????	

}

//???????
void Gui_StrCenter(uint16_t x, uint16_t y, uint8_t *str, uint8_t size, uint8_t mode)
{
    uint16_t x1;
    uint16_t len=strlen((const char *)str);
    if(size>16)
    {
        x1=(lcddev.width-len*(size/2))/2;
    }else
    {
        x1=(lcddev.width-len*8)/2;
    }
    
    Show_Str(x+x1,y,str,size,mode);
} 


void Load_Drow_Dialog(void)
{
    LCD_Clear(WHITE);//????   
    POINT_COLOR=BLUE;//????????????
    BACK_COLOR=WHITE;
    LCD_ShowString(lcddev.width-24,0,200,16,16,"RST");//???????????
    POINT_COLOR=RED;//?????????? 
}
////////////////////////////////////////////////////////////////////////////////
//???????????§Ó???
//??????
//x0,y0:????
//len:?????
//color:???
void gui_draw_hline(uint16_t x0,uint16_t y0,uint16_t len,uint16_t color)
{
    if(len==0)return;
    LCD_Fill(x0,y0,x0+len-1,y0,color);	
}
//??????
//x0,y0:????
//r:??
//color:???
void gui_fill_circle(uint16_t x0,uint16_t y0,uint16_t r,uint16_t color)
{											  
    uint32_t i;
    uint32_t imax = ((uint32_t)r*707)/1000+1;
    uint32_t sqmax = (uint32_t)r*(uint32_t)r+(uint32_t)r/2;
    uint32_t x=r;
    gui_draw_hline(x0-r,y0,2*r,color);
    for (i=1;i<=imax;i++) 
    {
        if ((i*i+x*x)>sqmax)// draw lines from outside  
        {
            if (x>imax) 
            {
                gui_draw_hline (x0-i+1,y0+x,2*(i-1),color);
                gui_draw_hline (x0-i+1,y0-x,2*(i-1),color);
            }
            x--;
        }
        // draw lines from inside (center)  
        gui_draw_hline(x0-x,y0+i,2*x,color);
        gui_draw_hline(x0-x,y0-i,2*x,color);
    }
}  

//?????????
//(x1,y1),(x2,y2):?????????????
//size?????????????
//color???????????
void lcd_draw_bline(uint16_t x1, uint16_t y1, uint16_t x2, uint16_t y2, uint8_t size, uint16_t color)
{
    uint16_t t; 
    int xerr=0,yerr=0,delta_x,delta_y,distance; 
    int incx,incy,uRow,uCol; 
    if(x1<size|| x2<size||y1<size|| y2<size)return; 
    delta_x=x2-x1; //???????????? 
    delta_y=y2-y1; 
    uRow=x1; 
    uCol=y1; 
    if(delta_x>0)incx=1; //??????????? 
    else if(delta_x==0)incx=0;//????? 
    else {incx=-1;delta_x=-delta_x;} 
    if(delta_y>0)incy=1; 
    else if(delta_y==0)incy=0;//???? 
    else{incy=-1;delta_y=-delta_y;} 
    if( delta_x>delta_y)distance=delta_x; //???????????????? 
    else distance=delta_y; 
    for(t=0;t<=distance+1;t++ )//??????? 
    {  
        gui_fill_circle(uRow,uCol,size,color);//???? 
        xerr+=delta_x ; 
        yerr+=delta_y ; 
        if(xerr>distance) 
        { 
            xerr-=distance; 
            uRow+=incx; 
        } 
        if(yerr>distance) 
        { 
            yerr-=distance; 
            uCol+=incy; 
        } 
    }  
}
