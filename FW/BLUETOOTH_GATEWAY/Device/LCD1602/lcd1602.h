#ifndef LCD1602_H_
#define LCD1602_H_

#ifdef __cplusplus
extern "C"
{
#endif

  /****************************************************************************
   *   PUBLIC FUNCTION
   ***************************************************************************/

  void LCD1602_Init(i2c_port_t e_i2c_port);
  void LCD1602_PrintString(i2c_port_t e_i2c_port, const char *str);
  void LCD1602_SetCursor(i2c_port_t e_i2c_port, uint8_t row, uint8_t col);
  void LCD1602_Set_Clear(i2c_port_t e_i2c_port);

#ifdef __cplusplus
}
#endif

#endif /* LCD1602_H_ */