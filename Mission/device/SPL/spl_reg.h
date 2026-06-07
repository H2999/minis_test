#ifndef SPL_REGISTER_MAP_H
#define SPL_REGISTER_MAP_H

//再多留意一下MEAS_CFG寄存器 可能有在读写数据前没检测的条件

#define SPL_ADDRESS (0x76 << 1)
#define SPL_PRESSURE_CFG 0x06
#define SPL_TEMPERATURE_CFG 0x07
#define SPL_MEAS_CFG 0x08
#define SPL_CFG_REG 0x09

#define SPL_PRESSURE_B2 0x00
#define SPL_PRESSURE_B1 0x01
#define SPL_PRESSURE_B0 0x02
#define SPL_TEMP_B2 0x03
#define SPL_TEMP_B1 0x04
#define SPL_TEMP_B0 0x05

#define SPL_INTERRUPT_STS 0x0A
#define SPL_FIFO_STS 0x0B
#define SPL_RESET 0x0C
#define SPL_Calibration_c0 0x10
#define SPL_Calibration_c0_c1 0x11
#define SPL_Calibration_c1 0x12
#define SPL_Calibration_c00_h 0x13
#define SPL_Calibration_c00_l 0x14
#define SPL_Calibration_c00_c10 0x15
#define SPL_Calibration_c10_h 0x16
#define SPL_Calibration_c10_l 0x17
#define SPL_Calibration_c01_h 0x18
#define SPL_Calibration_c01_l 0x19
#define SPL_Calibration_c11_h 0x1A
#define SPL_Calibration_c11_l 0x1B
#define SPL_Calibration_c20_h 0x1C
#define SPL_Calibration_c20_l 0x1D
#define SPL_Calibration_c21_h 0x1E
#define SPL_Calibration_c21_l 0x1F
#define SPL_Calibration_c30_h 0x20
#define SPL_Calibration_c30_l 0x21

#endif