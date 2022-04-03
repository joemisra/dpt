### dpt - multifunction electron temple

**NOTE: This is an early commit, example projects to follow.**

electrosmith daisy patch submodule based development platform

* 8 pots normalled to +5V.
* 8 CV in breaks normalled connection and pot acts as an attenuator.
* 2 gate in
* 2 gate out
* High fidelity stereo in/out (Daisy Patch Submodule uses an WM8731 codec).
* 2 12-bit CV out from Daisy Patch Submodule, scaled to -5v to 10v
* 4 12-bit CV (or lofi audio w/ some hacking) from external DAC7554, scaled to -7v to 7v.
* TRS MIDI in/out (in is auto-sensing for type a/b)
* SD card slot.
* Has an expander header on the back for +5v, +3v3, SPI, UART, I2C, and USB connections.

**This is primarily an internal project,  if you are interested in supplies/boards please get in touch.**

BOM

|Designator|Package                                                                                                                                                                                                                 |Quantity                                   |Designation|Supplier and ref      |
|----------|------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------|-------------------------------------------|-----------|----------------------|
|LED3,LED4 |3mm LED                                                                                                                                                                                                                 |2                                          |LED_3MM_RED|                      |
|J*|Mono Thonkiconn                                                                                                                                                                                                         |24                                         |fuzzySI_thonkiconn|                      |
|VRCV4,VRCV2,VRCV3,VRCV6,
VRCV5,VRCV1,VRCV8,VRCV7|9mm Potentiometer (Alpha / Alps)                                                                                                                                                                                        |8                                          |10K        |                      |
|D4,D6,D5,D3,D8,D7|3mm LED (Bipolar Red / Green)                                                                                                                                                                                           |6                                          |LED_Dual_Bidirectional|Tayda                 |
|U_SDCARD1 |VERT_MICROSD_CENTERED                                                                                                                                                                                                   |1                                          |MICRO_SD_CARDCENTERED| Mouser 945-PJS008U-3000-0
or Thonk |
|U9        |Electrosmith Daisy Patch Submodule                                                                                                                                                                                      |1                                          |ES_DAISY_PATCH_SM_REV1|                      |
|R36,R35,R18,R16,
R17,R19|R_0603_1608Metric                                                                                                                                                                                                       |6                                          |1k         |                      |
|U11,U5,U10,U3,
U6,U4|SOIC-8_3.9x4.9mm_P1.27mm                                                                                                                                                                                                |6                                          |TL072      |                      |
|C5,C4     |C_0603_1608Metric_Pad1.08x0.95mm_HandSolder                                                                                                                                                                             |2                                          |100n       |                      |
|R9,R11,R10,R8|R_0603_1608Metric                                                                                                                                                                                                       |4                                          |56k        |                      |
|C2,C3     |C_Elec_4x5.4                                                                                                                                                                                                            |2                                          |47u        |                      |
|J_I2C1    |PinHeader_2x03_P2.54mm_Vertical                                                                                                                                                                                         |1                                          |Conn_02x03_Odd_Even|                      |
|C11,C12,C13,C10,C18,C17|C_0603_1608Metric                                                                                                                                                                                                       |6                                          |1n         |                      |
|R22,R4,R7,R20,R29,
R30,R2,R6,R5|R_0603_1608Metric                                                                                                                                                                                                       |9                                          |10k        |                      |
|C14,C16,C6,C7,
C1,C8,C15,C9|C_0603_1608Metric                                                                                                                                                                                                       |8                                          |100n       |                      |
|R26,R34,R15,R24,
R33,R28,R27,R14,
R13,R25,R12|R_0603_1608Metric                                                                                                                                                                                                       |11                                         |220R       |                      |
|J14       |PinHeader_2x05_P2.54mm_Vertical                                                                                                                                                                                         |1                                          |Conn_02x05_Odd_Even|                      |
|U2        |MSOP-10_3x3mm_P0.5mm                                                                                                                                                                                                    |1                                          |DAC7554IDGS|                      |
|R32,R31   |R_0603_1608Metric                                                                                                                                                                                                       |2                                          |30k        |                      |
|R23,R21   |R_0603_1608Metric                                                                                                                                                                                                       |2                                          |4.7k       |                      |
|U8        |SOIC-8_3.9x4.9mm_P1.27mm                                                                                                                                                                                                |1                                          |HCPL-0631  |                      |
|U1        |SOT-23                                                                                                                                                                                                                  |1                                          |LM4040DBZ-2.5|                      |
|J_EXPANDER2|PinHeader_2x08_P2.54mm_Vertical                                                                                                                                                                                         |1                                          |USB_SPI_MUX_EXPANDER|                      |
|R1        |R_0603_1608Metric                                                                                                                                                                                                       |1                                          |3k         |                      |
|D2,D1     |D_SOD-323                                                                                                                                                                                                               |2                                          |1N1517     |                      |
|R3        |R_0603_1608Metric                                                                                                                                                                                                       |1                                          |7.5k       |                      |
|U7        |SOT-23-5                                                                                                                                                                                                                |1                                          |SN74LVC1G17DBV|                      |


![dpt](dpt.jpg)
