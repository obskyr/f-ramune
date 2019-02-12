EESchema Schematic File Version 2
LIBS:f-ramune
LIBS:teensy
LIBS:power
LIBS:device
LIBS:switches
LIBS:relays
LIBS:motors
LIBS:transistors
LIBS:conn
LIBS:linear
LIBS:regul
LIBS:74xx
LIBS:cmos4000
LIBS:adc-dac
LIBS:memory
LIBS:xilinx
LIBS:microcontrollers
LIBS:dsp
LIBS:microchip
LIBS:analog_switches
LIBS:motorola
LIBS:texas
LIBS:intel
LIBS:audio
LIBS:interface
LIBS:digital-audio
LIBS:philips
LIBS:display
LIBS:cypress
LIBS:siliconi
LIBS:opto
LIBS:atmel
LIBS:contrib
LIBS:valves
EELAYER 25 0
EELAYER END
$Descr A4 11693 8268
encoding utf-8
Sheet 1 1
Title ""
Date ""
Rev ""
Comp ""
Comment1 ""
Comment2 ""
Comment3 ""
Comment4 ""
$EndDescr
$Comp
L FM1808B-SG U3
U 1 1 5C51655B
P 8100 2500
F 0 "U3" H 7800 3350 50  0000 C CNN
F 1 "FM1808B-SG" H 8400 1650 50  0000 C CNN
F 2 "" H 8100 2500 50  0001 C CNN
F 3 "" H 8100 2500 50  0001 C CNN
	1    8100 2500
	1    0    0    -1  
$EndComp
$Comp
L GNDREF #PWR01
U 1 1 5C516614
P 8100 3500
F 0 "#PWR01" H 8100 3250 50  0001 C CNN
F 1 "GNDREF" H 8100 3350 50  0000 C CNN
F 2 "" H 8100 3500 50  0001 C CNN
F 3 "" H 8100 3500 50  0001 C CNN
	1    8100 3500
	1    0    0    -1  
$EndComp
Wire Wire Line
	8100 3400 8100 3500
$Comp
L Teensy3.2 U1
U 1 1 5C5169CB
P 2950 2650
F 0 "U1" H 2950 4150 60  0000 C CNN
F 1 "Teensy3.2" H 2950 1150 60  0000 C CNN
F 2 "Housings_DIP:DIP-28_W7.62mm_Socket_LongPads" H 2950 1050 60  0001 C CNN
F 3 "" H 2950 1850 60  0000 C CNN
	1    2950 2650
	1    0    0    -1  
$EndComp
Text GLabel 1850 1450 0    60   Output ~ 0
DS
Text GLabel 1850 1550 0    60   Output ~ 0
ST_CP
Text GLabel 1850 1650 0    60   Output ~ 0
SH_CP
Text GLabel 1850 1750 0    60   Output ~ 0
~WE
Text GLabel 1850 1850 0    60   Output ~ 0
~OE
Text GLabel 1850 1950 0    60   Output ~ 0
~CE
Text GLabel 1850 2250 0    60   Output ~ 0
A8
Text GLabel 1850 2350 0    60   Output ~ 0
A9
Text GLabel 1850 2450 0    60   Output ~ 0
A10
Text GLabel 1850 2550 0    60   Output ~ 0
A11
Text GLabel 1850 2650 0    60   Output ~ 0
A12
Text GLabel 1850 3250 0    60   Output ~ 0
A13
Text GLabel 1850 3350 0    60   Output ~ 0
A14
Text GLabel 1850 3550 0    60   BiDi ~ 0
D0
Text GLabel 1850 3650 0    60   BiDi ~ 0
D1
Text GLabel 1850 3750 0    60   BiDi ~ 0
D2
Text GLabel 1850 3850 0    60   BiDi ~ 0
D3
Text GLabel 1850 3950 0    60   BiDi ~ 0
D4
Text GLabel 4050 3950 2    60   BiDi ~ 0
D5
Text GLabel 4050 3850 2    60   BiDi ~ 0
D6
Text GLabel 4050 3750 2    60   BiDi ~ 0
D7
Wire Wire Line
	3950 3750 4050 3750
Wire Wire Line
	3950 3850 4050 3850
Wire Wire Line
	4050 3950 3950 3950
Wire Wire Line
	1950 3950 1850 3950
Wire Wire Line
	1950 3850 1850 3850
Wire Wire Line
	1950 3750 1850 3750
Wire Wire Line
	1950 3650 1850 3650
Wire Wire Line
	1850 3550 1950 3550
Wire Wire Line
	1950 3350 1850 3350
Wire Wire Line
	1850 3250 1950 3250
Wire Wire Line
	1950 2650 1850 2650
Wire Wire Line
	1850 2550 1950 2550
Wire Wire Line
	1950 2450 1850 2450
Wire Wire Line
	1850 2350 1950 2350
Wire Wire Line
	1950 2250 1850 2250
Wire Wire Line
	1850 1450 1950 1450
Wire Wire Line
	1950 1550 1850 1550
Wire Wire Line
	1850 1650 1950 1650
Wire Wire Line
	1950 1750 1850 1750
Wire Wire Line
	1850 1850 1950 1850
Wire Wire Line
	1950 1950 1850 1950
$Comp
L GNDREF #PWR02
U 1 1 5C51766E
P 1300 1350
F 0 "#PWR02" H 1300 1100 50  0001 C CNN
F 1 "GNDREF" H 1300 1200 50  0000 C CNN
F 2 "" H 1300 1350 50  0001 C CNN
F 3 "" H 1300 1350 50  0001 C CNN
	1    1300 1350
	1    0    0    -1  
$EndComp
Wire Wire Line
	1300 1350 1950 1350
$Comp
L +5V #PWR03
U 1 1 5C517901
P 8100 1500
F 0 "#PWR03" H 8100 1350 50  0001 C CNN
F 1 "+5V" H 8100 1640 50  0000 C CNN
F 2 "" H 8100 1500 50  0001 C CNN
F 3 "" H 8100 1500 50  0001 C CNN
	1    8100 1500
	1    0    0    -1  
$EndComp
Wire Wire Line
	8100 1500 8100 1600
Text GLabel 8700 1800 2    60   BiDi ~ 0
D0
Text GLabel 8700 1900 2    60   BiDi ~ 0
D1
Text GLabel 8700 2000 2    60   BiDi ~ 0
D2
Text GLabel 8700 2100 2    60   BiDi ~ 0
D3
Text GLabel 8700 2200 2    60   BiDi ~ 0
D4
Text GLabel 8700 2300 2    60   BiDi ~ 0
D5
Text GLabel 8700 2400 2    60   BiDi ~ 0
D6
Text GLabel 8700 2500 2    60   BiDi ~ 0
D7
Wire Wire Line
	8700 1800 8600 1800
Wire Wire Line
	8600 1900 8700 1900
Wire Wire Line
	8700 2000 8600 2000
Wire Wire Line
	8600 2100 8700 2100
Wire Wire Line
	8700 2200 8600 2200
Wire Wire Line
	8600 2300 8700 2300
Wire Wire Line
	8700 2400 8600 2400
Wire Wire Line
	8600 2500 8700 2500
Text GLabel 8700 2900 2    60   Input ~ 0
~OE
Text GLabel 8700 3000 2    60   Input ~ 0
~WE
Text GLabel 8700 3200 2    60   Input ~ 0
~CE
Text GLabel 7500 1800 0    60   Input ~ 0
A0
Text GLabel 7500 1900 0    60   Input ~ 0
A1
Text GLabel 7500 2000 0    60   Input ~ 0
A2
Text GLabel 7500 2200 0    60   Input ~ 0
A4
Text GLabel 7500 2100 0    60   Input ~ 0
A3
Text GLabel 7500 2300 0    60   Input ~ 0
A5
Text GLabel 7500 2400 0    60   Input ~ 0
A6
Text GLabel 7500 2500 0    60   Input ~ 0
A7
Text GLabel 7500 2600 0    60   Input ~ 0
A8
Text GLabel 7500 2700 0    60   Input ~ 0
A9
Text GLabel 7500 2800 0    60   Input ~ 0
A10
Text GLabel 7500 2900 0    60   Input ~ 0
A11
Text GLabel 7500 3000 0    60   Input ~ 0
A12
Text GLabel 7500 3100 0    60   Input ~ 0
A13
Text GLabel 7500 3200 0    60   Input ~ 0
A14
Wire Wire Line
	7500 1800 7600 1800
Wire Wire Line
	7500 1900 7600 1900
Wire Wire Line
	7500 2000 7600 2000
Wire Wire Line
	7500 2100 7600 2100
Wire Wire Line
	7500 2200 7600 2200
Wire Wire Line
	7500 2300 7600 2300
Wire Wire Line
	7500 2400 7600 2400
Wire Wire Line
	7500 2500 7600 2500
Wire Wire Line
	7500 2600 7600 2600
Wire Wire Line
	7500 2700 7600 2700
Wire Wire Line
	7500 2800 7600 2800
Wire Wire Line
	7500 2900 7600 2900
Wire Wire Line
	7500 3000 7600 3000
Wire Wire Line
	7500 3100 7600 3100
Wire Wire Line
	7500 3200 7600 3200
Wire Wire Line
	8600 2900 8700 2900
Wire Wire Line
	8700 3000 8600 3000
Wire Wire Line
	8600 3200 8700 3200
Text GLabel 6700 2150 2    60   Output ~ 0
A0
Text GLabel 6700 2250 2    60   Output ~ 0
A1
Text GLabel 6700 2350 2    60   Output ~ 0
A2
Text GLabel 6700 2450 2    60   Output ~ 0
A3
Text GLabel 6700 2550 2    60   Output ~ 0
A4
Text GLabel 6700 2650 2    60   Output ~ 0
A5
Text GLabel 6700 2750 2    60   Output ~ 0
A6
Text GLabel 6700 2850 2    60   Output ~ 0
A7
Text GLabel 5100 2150 0    60   Input ~ 0
DS
Text GLabel 5100 2350 0    60   Input ~ 0
SH_CP
Text GLabel 5100 2650 0    60   Input ~ 0
ST_CP
Wire Wire Line
	5100 2650 5200 2650
Wire Wire Line
	5100 2350 5200 2350
$Comp
L GNDREF #PWR04
U 1 1 5C518418
P 4600 2750
F 0 "#PWR04" H 4600 2500 50  0001 C CNN
F 1 "GNDREF" H 4600 2600 50  0000 C CNN
F 2 "" H 4600 2750 50  0001 C CNN
F 3 "" H 4600 2750 50  0001 C CNN
	1    4600 2750
	1    0    0    -1  
$EndComp
$Comp
L +5V #PWR05
U 1 1 5C518432
P 4600 2450
F 0 "#PWR05" H 4600 2300 50  0001 C CNN
F 1 "+5V" H 4600 2590 50  0000 C CNN
F 2 "" H 4600 2450 50  0001 C CNN
F 3 "" H 4600 2450 50  0001 C CNN
	1    4600 2450
	1    0    0    -1  
$EndComp
Wire Wire Line
	5200 2450 4600 2450
Wire Wire Line
	5200 2750 4600 2750
Wire Wire Line
	5200 2150 5100 2150
Wire Wire Line
	6600 2850 6700 2850
Wire Wire Line
	6700 2750 6600 2750
Wire Wire Line
	6600 2650 6700 2650
Wire Wire Line
	6700 2550 6600 2550
Wire Wire Line
	6600 2450 6700 2450
Wire Wire Line
	6700 2350 6600 2350
Wire Wire Line
	6600 2250 6700 2250
Wire Wire Line
	6700 2150 6600 2150
$Comp
L +5V #PWR06
U 1 1 5C5189C1
P 4350 3450
F 0 "#PWR06" H 4350 3300 50  0001 C CNN
F 1 "+5V" H 4350 3590 50  0000 C CNN
F 2 "" H 4350 3450 50  0001 C CNN
F 3 "" H 4350 3450 50  0001 C CNN
	1    4350 3450
	1    0    0    -1  
$EndComp
Wire Wire Line
	3950 3450 4350 3450
NoConn ~ 3950 1350
NoConn ~ 3950 1450
NoConn ~ 3950 1550
NoConn ~ 3950 1650
NoConn ~ 3950 1750
NoConn ~ 3950 1850
NoConn ~ 3950 1950
NoConn ~ 3950 2050
NoConn ~ 3950 2150
NoConn ~ 3950 2250
NoConn ~ 3950 2350
NoConn ~ 3950 2450
NoConn ~ 3950 2550
NoConn ~ 3950 2650
NoConn ~ 3950 2750
NoConn ~ 3950 2850
NoConn ~ 3950 2950
NoConn ~ 3950 3050
NoConn ~ 3950 3150
NoConn ~ 3950 3250
NoConn ~ 3950 3350
NoConn ~ 3950 3550
NoConn ~ 3950 3650
NoConn ~ 1950 3450
NoConn ~ 1950 3150
NoConn ~ 1950 3050
NoConn ~ 1950 2950
NoConn ~ 1950 2850
NoConn ~ 1950 2750
NoConn ~ 1950 2050
NoConn ~ 1950 2150
NoConn ~ 6600 3050
$Comp
L PWR_FLAG #FLG07
U 1 1 5C5195C4
P 4200 3500
F 0 "#FLG07" H 4200 3575 50  0001 C CNN
F 1 "PWR_FLAG" H 4200 3650 50  0000 C CNN
F 2 "" H 4200 3500 50  0001 C CNN
F 3 "" H 4200 3500 50  0001 C CNN
	1    4200 3500
	1    0    0    1   
$EndComp
$Comp
L PWR_FLAG #FLG08
U 1 1 5C5196D9
P 1650 1300
F 0 "#FLG08" H 1650 1375 50  0001 C CNN
F 1 "PWR_FLAG" H 1650 1450 50  0000 C CNN
F 2 "" H 1650 1300 50  0001 C CNN
F 3 "" H 1650 1300 50  0001 C CNN
	1    1650 1300
	1    0    0    -1  
$EndComp
Wire Wire Line
	4200 3500 4200 3450
Connection ~ 4200 3450
Wire Wire Line
	1650 1300 1650 1350
Connection ~ 1650 1350
$Comp
L 74HC595 U2
U 1 1 5C51A80B
P 5900 2600
F 0 "U2" H 6050 3200 50  0000 C CNN
F 1 "74HC595" H 6100 2000 50  0000 C CNN
F 2 "" H 5900 2600 50  0001 C CNN
F 3 "" H 5900 2600 50  0001 C CNN
	1    5900 2600
	1    0    0    -1  
$EndComp
$Comp
L +5V #PWR09
U 1 1 5C51A866
P 5900 1850
F 0 "#PWR09" H 5900 1700 50  0001 C CNN
F 1 "+5V" H 5900 1990 50  0000 C CNN
F 2 "" H 5900 1850 50  0001 C CNN
F 3 "" H 5900 1850 50  0001 C CNN
	1    5900 1850
	1    0    0    -1  
$EndComp
$Comp
L GNDREF #PWR010
U 1 1 5C51A8D1
P 5900 3350
F 0 "#PWR010" H 5900 3100 50  0001 C CNN
F 1 "GNDREF" H 5900 3200 50  0000 C CNN
F 2 "" H 5900 3350 50  0001 C CNN
F 3 "" H 5900 3350 50  0001 C CNN
	1    5900 3350
	1    0    0    -1  
$EndComp
Wire Wire Line
	5900 1850 5900 1950
Wire Wire Line
	5900 3250 5900 3350
$EndSCHEMATC
