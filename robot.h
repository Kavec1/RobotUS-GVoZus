#include "init.h"
//kniznica obsahujuca USART spojenie
#include "USART.h"
#include "motory.h"
#include "ultrazvuk.h"
#define CAS10 255	//kazdych 10 ms bude volana obsluha prerusenia TC0
#define REZIM 0b00001101 //CTC rezim najpomalsie hodiny F_CPU/1024
