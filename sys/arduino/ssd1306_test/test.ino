


#include <Arduino.h>
#include <SPI.h>
#include "u8x8.h"



/*=============================================*/
/* callbacks */

extern "C" uint8_t u8x8_gpio_and_delay_arduino(u8x8_t *u8x8, uint8_t msg, uint8_t arg_int, void *arg_ptr)
{
  uint8_t i;
  switch(msg)
  {
    case U8X8_MSG_GPIO_AND_DELAY_INIT:
    
      for( i = 0; i < U8X8_PIN_CNT; i++ )
	if ( u8x8->pins[i] != U8X8_PIN_NONE )
	  pinMode(u8x8->pins[i], OUTPUT);
	  
      break;
  
    case U8X8_MSG_DELAY_MILLI:
      delay(arg_int);
      break;
    case U8X8_MSG_DELAY_I2C:
      /* arg_int is 1 or 4: 100KHz (5us) or 400KHz (1.25us) */
      delayMicroseconds(arg_int<=2?5:2);
      break;
    case U8X8_MSG_GPIO_I2C_CLOCK:
    case U8X8_MSG_GPIO_I2C_DATA:
      if ( arg_int == 0 )
      {
	pinMode(u8x8_GetPinValue(u8x8, msg), OUTPUT);
	digitalWrite(u8x8_GetPinValue(u8x8, msg), 0);
      }
      else
      {
	pinMode(u8x8_GetPinValue(u8x8, msg), INPUT_PULLUP);
      }
      break;
    default:
      if ( msg >= U8X8_MSG_GPIO(0) )
      {
	i = u8x8_GetPinValue(u8x8, msg);
	if ( i != U8X8_PIN_NONE )
	  digitalWrite(i, arg_int);
	break;
      }
      
      return 0;
  }
  return 1;
}


/*=============================================*/

extern "C" uint8_t u8x8_byte_arduino_hw_spi(u8x8_t *u8g2, uint8_t msg, uint8_t arg_int, void *arg_ptr)
{
  uint8_t *data;
 
  switch(msg)
  {
    case U8X8_MSG_BYTE_SEND:
      data = (uint8_t *)arg_ptr;
      while( arg_int > 0 )
      {
	SPI.transfer((uint8_t)*data);
	data++;
	arg_int--;
      }
      break;
    case U8X8_MSG_BYTE_INIT:
      /* disable chipselect */
      u8x8_gpio_SetCS(u8g2, u8g2->display_info->chip_disable_level);
      /* no wait required here */
      
      /* for SPI: setup correct level of the clock signal */
      pinMode(11, OUTPUT);
      pinMode(13, OUTPUT);
      digitalWrite(13, u8g2->display_info->sck_takeover_edge);
      break;
      
    case U8X8_MSG_BYTE_SET_DC:
      u8x8_gpio_SetDC(u8g2, arg_int);
      break;
      
    case U8X8_MSG_BYTE_START_TRANSFER:
      SPI.begin();
      
      if ( u8g2->display_info->sck_pulse_width_ns < 70 )
	SPI.setClockDivider( SPI_CLOCK_DIV2 );
      else
	SPI.setClockDivider( SPI_CLOCK_DIV4 );
      SPI.setDataMode(SPI_MODE0);
      SPI.setBitOrder(MSBFIRST);
      
      u8x8_gpio_SetCS(u8g2, u8g2->display_info->chip_enable_level);  
      u8g2->gpio_and_delay_cb(u8g2, U8X8_MSG_DELAY_NANO, u8g2->display_info->post_chip_enable_wait_ns, NULL);
      break;
      
    case U8X8_MSG_BYTE_END_TRANSFER:      
      u8g2->gpio_and_delay_cb(u8g2, U8X8_MSG_DELAY_NANO, u8g2->display_info->pre_chip_disable_wait_ns, NULL);
      u8x8_gpio_SetCS(u8g2, u8g2->display_info->chip_disable_level);

      SPI.end();

      break;
    case U8X8_MSG_BYTE_SET_I2C_ADR:
      break;
    case U8X8_MSG_BYTE_SET_DEVICE:
      break;
    default:
      return 0;
  }
  return 1;
}

/*=============================================*/
/* interface setup procedures */

/*
u8x8_SetPin(u8x8,pin,val)

values for "pin":
U8X8_PIN_D0
U8X8_PIN_SPI_CLOCK
U8X8_PIN_D1
U8X8_PIN_SPI_DATA
U8X8_PIN_D2
U8X8_PIN_D3
U8X8_PIN_D4
U8X8_PIN_D5
U8X8_PIN_D6
U8X8_PIN_D7
U8X8_PIN_E
U8X8_PIN_CS
U8X8_PIN_DC
U8X8_PIN_RESET

values vor "val":
any number or:
U8X8_PIN_NONE
*/


/*
  use U8X8_PIN_NONE as value for "reset", if there is no reset line
*/
void u8x8_Setup_4Wire_SW_SPI(u8x8_t *u8x8, u8x8_msg_cb display_cb, uint8_t clock, uint8_t data, uint8_t cs, uint8_t dc, uint8_t reset)
{
  u8x8_Setup(u8x8, display_cb, u8x8_cad_001, u8x8_byte_4wire_sw_spi, u8x8_gpio_and_delay_arduino);
  
  /* assign individual pin values (only for ARDUINO, if pin_list is available) */
  u8x8_SetPin(u8x8, U8X8_PIN_SPI_CLOCK, clock);
  u8x8_SetPin(u8x8, U8X8_PIN_SPI_DATA, data);
  u8x8_SetPin(u8x8, U8X8_PIN_CS, cs);
  u8x8_SetPin(u8x8, U8X8_PIN_DC, dc);
  u8x8_SetPin(u8x8, U8X8_PIN_RESET, reset);
}


void u8x8_Setup_3Wire_SW_SPI(u8x8_t *u8x8, u8x8_msg_cb display_cb, uint8_t clock, uint8_t data, uint8_t cs, uint8_t reset)
{
  u8x8_Setup(u8x8, display_cb, u8x8_cad_001, u8x8_byte_3wire_sw_spi, u8x8_gpio_and_delay_arduino);
  
  /* assign individual pin values (only for ARDUINO, if pin_list is available) */
  u8x8_SetPin(u8x8, U8X8_PIN_SPI_CLOCK, clock);
  u8x8_SetPin(u8x8, U8X8_PIN_SPI_DATA, data);
  u8x8_SetPin(u8x8, U8X8_PIN_CS, cs);
  u8x8_SetPin(u8x8, U8X8_PIN_RESET, reset);
}

/*
  use U8X8_PIN_NONE as value for "reset", if there is no reset line
*/
void u8x8_Setup_4Wire_HW_SPI(u8x8_t *u8x8, u8x8_msg_cb display_cb, uint8_t cs, uint8_t dc, uint8_t reset)
{
  u8x8_Setup(u8x8, display_cb, u8x8_cad_001, u8x8_byte_arduino_hw_spi, u8x8_gpio_and_delay_arduino);
  
  /* assign individual pin values (only for ARDUINO, if pin_list is available) */
  u8x8_SetPin(u8x8, U8X8_PIN_CS, cs);
  u8x8_SetPin(u8x8, U8X8_PIN_DC, dc);
  u8x8_SetPin(u8x8, U8X8_PIN_RESET, reset);
}

void u8x8_Setup_SSD13xx_SW_I2C(u8x8_t *u8x8, u8x8_msg_cb display_cb, uint8_t clock, uint8_t data, uint8_t reset)
{
  u8x8_Setup(u8x8, display_cb, u8x8_cad_001, u8x8_byte_ssd13xx_sw_i2c, u8x8_gpio_and_delay_arduino);
  
  /* assign individual pin values (only for ARDUINO, if pin_list is available) */
  u8x8_SetPin(u8x8, U8X8_PIN_I2C_CLOCK, clock);
  u8x8_SetPin(u8x8, U8X8_PIN_I2C_DATA, data);
  u8x8_SetPin(u8x8, U8X8_PIN_RESET, reset);

}

void u8x8_Setup_8Bit_6800(u8x8_t *u8x8, u8x8_msg_cb display_cb, uint8_t d0, uint8_t d1, uint8_t d2, uint8_t d3, uint8_t d4, uint8_t d5, uint8_t d6, uint8_t d7, uint8_t enable, uint8_t cs, uint8_t dc, uint8_t reset)
{
  u8x8_Setup(u8x8, display_cb, u8x8_cad_001, u8x8_byte_8bit_6800mode, u8x8_gpio_and_delay_arduino);
  
  /* assign individual pin values (only for ARDUINO, if pin_list is available) */
  u8x8_SetPin(u8x8, U8X8_PIN_D0, d0);
  u8x8_SetPin(u8x8, U8X8_PIN_D1, d1);
  u8x8_SetPin(u8x8, U8X8_PIN_D2, d2);
  u8x8_SetPin(u8x8, U8X8_PIN_D3, d3);
  u8x8_SetPin(u8x8, U8X8_PIN_D4, d4);
  u8x8_SetPin(u8x8, U8X8_PIN_D5, d5);
  u8x8_SetPin(u8x8, U8X8_PIN_D6, d6);
  u8x8_SetPin(u8x8, U8X8_PIN_D7, d7);
  u8x8_SetPin(u8x8, U8X8_PIN_E, enable);
  u8x8_SetPin(u8x8, U8X8_PIN_CS, cs);
  u8x8_SetPin(u8x8, U8X8_PIN_DC, dc);
  u8x8_SetPin(u8x8, U8X8_PIN_RESET, reset);
}

void u8x8_Setup_8Bit_8080(u8x8_t *u8x8, u8x8_msg_cb display_cb, uint8_t d0, uint8_t d1, uint8_t d2, uint8_t d3, uint8_t d4, uint8_t d5, uint8_t d6, uint8_t d7, uint8_t wr, uint8_t cs, uint8_t dc, uint8_t reset)
{
  u8x8_Setup(u8x8, display_cb, u8x8_cad_001, u8x8_byte_8bit_8080mode, u8x8_gpio_and_delay_arduino);
  
  /* assign individual pin values (only for ARDUINO, if pin_list is available) */
  u8x8_SetPin(u8x8, U8X8_PIN_D0, d0);
  u8x8_SetPin(u8x8, U8X8_PIN_D1, d1);
  u8x8_SetPin(u8x8, U8X8_PIN_D2, d2);
  u8x8_SetPin(u8x8, U8X8_PIN_D3, d3);
  u8x8_SetPin(u8x8, U8X8_PIN_D4, d4);
  u8x8_SetPin(u8x8, U8X8_PIN_D5, d5);
  u8x8_SetPin(u8x8, U8X8_PIN_D6, d6);
  u8x8_SetPin(u8x8, U8X8_PIN_D7, d7);
  u8x8_SetPin(u8x8, U8X8_PIN_E, wr);
  u8x8_SetPin(u8x8, U8X8_PIN_CS, cs);
  u8x8_SetPin(u8x8, U8X8_PIN_DC, dc);
  u8x8_SetPin(u8x8, U8X8_PIN_RESET, reset);
}


/*=============================================*/
/* display setup procedures */


u8x8_t u8x8;
uint8_t tile[8] = { 0x0f, 0x0f, 0x0f, 0x0f, 0xf0, 0xf0, 0xf0, 0xf0 };

void setup(void)
{
  pinMode(7, OUTPUT);
  pinMode(9, OUTPUT);
  pinMode(10, OUTPUT);
  digitalWrite(7, 0);	// default output in I2C mode for the SSD1306 test shield
  digitalWrite(9, 0);	// default output in I2C mode for the SSD1306 test shield: set the i2c adr to 0
  digitalWrite(10, 0);	// default output in I2C mode for the SSD1306 test shield
  
  //u8x8_Setup_3Wire_SW_SPI(&u8x8, u8x8_d_ssd1306_128x64_noname, 13, 11, 10, 8);
  //u8x8_Setup_4Wire_SW_SPI(&u8x8, u8x8_d_ssd1306_128x64_noname, 13, 11, 10, 9, 8);
  //u8x8_Setup_4Wire_HW_SPI(&u8x8, u8x8_d_ssd1306_128x64_noname, 10, 9, 8);

  u8x8_Setup_SSD13xx_SW_I2C(&u8x8, u8x8_d_ssd1306_128x64_noname, 13, 11, 8);
  //u8x8_Setup_8Bit_6800(&u8x8, u8x8_d_ssd1306_128x64_noname, 13, 11, 2, 3, 4, 5, 6, A4, /*enable=*/ 7, /*cs=*/ 10, /*dc=*/ 9, /*reset=*/ 8);
  //u8x8_Setup_8Bit_8080(&u8x8, u8x8_d_ssd1306_128x64_noname, 13, 11, 2, 3, 4, 5, 6, A4, /*enable=*/ 7, /*cs=*/ 10, /*dc=*/ 9, /*reset=*/ 8);
  
  //u8x8_Setup_4Wire_SW_SPI(&u8x8, u8x8_d_uc1701_dogs102, 13, 11, 10, 9, 8);
  //u8x8_Setup_4Wire_HW_SPI(&u8x8, u8x8_d_uc1701_dogs102, 10, 9, 8);		
}

void loop(void)
{
  u8x8_InitDisplay(&u8x8);  
  //u8x8_SetFlipMode(&u8x8, 1);

  //digitalWrite(9, 0);	// default output in I2C mode for the SSD1306 test shield: set the i2c adr to 0

  for(;;)
  {
    u8x8_ClearScreen(&u8x8);  
    u8x8_SetPowerSave(&u8x8, 0);
    //u8x8_SetContrast(&u8x8, 10);
    
    //delay(500);

    u8x8_SetFont(&u8x8, u8x8_font_chroma48medium8_r);
    u8x8_DrawString(&u8x8, 0, 0, "Hello World");
    u8x8_DrawString(&u8x8, 3, 1, "ABCdefg");
    
    u8x8_DrawTile(&u8x8, 1, 1, 1, tile);
    u8x8_DrawTile(&u8x8, 2, 2, 1, tile);
    u8x8_DrawTile(&u8x8, 3, 3, 1, tile);
    u8x8_DrawTile(&u8x8, 4, 4, 1, tile);
    u8x8_DrawTile(&u8x8, 5, 5, 1, tile);
    u8x8_DrawTile(&u8x8, 6, 6, 1, tile);

    //delay(2000);
  }
}

