# Mailman
Atttiny85/Digispark (Digistump) based project to notify me if my phisical mailbox was opene (i.e. mailman put something in it).

Having new house for a year I still don't have a habit to check my mailbox frequently. So I thought I can build a circuit to notify my when someone have opened the slot (and probably put something to my mailbox).

So the plan was prepared:
- the thing will be powered by Li-Ion cell (I have some recycled from dead laptop battery packs)
- it should take as little power as possible (to extend the battery life)
- it should be triggered by physical switch (activated by moved mailbox slot cover)
- it should blink several times high brightness LED every few seconds after mailbox was opened
- it should blink once every few seconds if the battery is low (I assumed about 3V)
- future expansion: it should transmit the notification using some radio, probably nRF24L01 (I have some in my drawer and it's pretty low power in sleep mode) to notify "smart home" (I use Souliss + OpenHAB)

I have chosen ATTiny processor in the form of Digistump (Digispark) Chinese clone. They are cheap and easy to play with using Arduino IDE.
I removed both power LED and voltage regulator from the board to remove unecessary power loss. Of course I can't power the board with voltage over 5V anymore, but it works perfecly when powered by 1 li-ion (via Vcc pin) or via USB (NOTE: not in the same time! Connecting 5V directly to li-ion could burn the cell!). I left USB circuit intact.
The supply current is now about 1.75mA in a sleep mode.

Pins used:
a) GND
b) Vcc (connected directly to li-ion)
c) P1 - LED (I left on board LED intact and added NPN transistor to activate external LED). High means LED is lit.
d) P0 - close to ground (LOW) when mailbox cover is open

How it works:
1. When powered, bootloader waits about 5s for programming request and then passes the execution to our code.
2. LED blinks slowly three times to let us know everything is OK.
3. In about 20 seconds the supply voltage is checked and if it's OK the device go to sleep mode. If battery voltage is low it starts blinking briefly every few seconds until reset or battery charge happens. Voltage is checked every 15 - 30 minutes.
4. If mailbox is open LED starts blinking 3 times every few seconds until reset (power on-off).

The code is public domain (let's say: LGPL)  and, as you can expect, use it as you wish and on your own responsibility. I would be very happy if you find it useful.
All suggestions on code, idea or circuits are very welcome!

Kind regards,
Jarek Andrzejewski
ptja@ptja.pl
+48 601 324634
