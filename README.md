# Arduino EEPROM Programmer

Arduino based EEPROM (AT28C16) programmer. Based on the work of [Ben Eater](https://eater.net/). 

![image](https://user-images.githubusercontent.com/1328048/209965494-b23d4c75-d243-4cf3-abdc-37b5514308e8.png)

The EEPROM is an [AT28C16 (16K)](https://www.mouser.ca/datasheet/2/268/doc0540-1180724.pdf).

Due to the limited number of output pins on the Arduino, shift registers are needed. The specific shift registers are [SN 74HC 595N](https://www.ti.com/lit/ds/symlink/sn54hc595-sp.pdf?HQS=dis-mous-null-mousermode-dsf-pf-null-wwe&ts=1672311037635&ref_url=https%253A%252F%252Fwww.mouser.co.il%252F). 
