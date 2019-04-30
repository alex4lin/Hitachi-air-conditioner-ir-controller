# Hitachi-air-conditioner-ir-controller
Hitachi air conditioner ir controller (PN:RF07T1) based on ir-slinger

It is a tool to simulate an ir controller of Hitachi air conditioner.


1. select the available GPIO port (CPIO17) and connect the ir_diodes with it. 
https://www.raspberrypi-spy.co.uk/2012/06/simple-guide-to-the-rpi-gpio-header-and-pins/

2. Compile it by gcc.
gcc Hitachi.c -lm -lpigpio -pthread -lrt -o Hitachi

3. Run the tool with sudo & assign parameters.
sudo ./Hitachi key temperature wind mode power
               key: 0x13 start/stop, 0x22 reserve, 0x24 cancel, 0x31 sleep,
                    0x41 mode, 0x42 wind speed, 0x43 temp down, 0x44 temp up,
                    0x71 dehumidify, 0x81 auto wind direction, 0x89 quick
               temperature: 16~32 degree
               wind: 1 silent, 2 slight, 3 weak, 4 strong, 5 auto 
               mode: 1 air supply, 3 cold, 5 dehumidify, 6 warm,  7 auto control
               power: 1 on, 0 off 

It is based on the ir-slinger library. Refer below link.
https://github.com/bschwind/ir-slinger

Due to the encoding length of Hitachi ir code over the default setting of pigpio, it is MUST to expand the buffer.
pigpio.h =>PI_WAVE_MAX_PULSES 4*3000 to 4*3100
pigpio.c => PAGES_PER_BLOCK 53 to 55