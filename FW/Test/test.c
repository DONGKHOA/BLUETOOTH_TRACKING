#include <stdio.h>
#include <stdint.h>

void
Process_Data ()
{
  int Bytes = BSP_uartReadByte(uart_port_num, p_received_package, 1);

  if (Bytes > 0)
  {
    if (p_received_package[0] == 0xef)
    {
      if (p_received_package[1] == 0x01) //Check heading
      {
      }
    }
  }
}
