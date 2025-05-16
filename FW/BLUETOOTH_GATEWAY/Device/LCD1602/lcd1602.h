#ifndef LCD1602_H_
#define LCD1602_H_

#ifdef __cplusplus
extern "C"
{
#endif

  /****************************************************************************
   *   PUBLIC FUNCTION
   ***************************************************************************/

  void LCD1602_Init(void);
  void LCD1602_SetCursor(uint8_t row, uint8_t col);
  void LCD1602_PrintString(const char *str);

#ifdef __cplusplus
}
#endif

#endif /* LCD1602_H_ */