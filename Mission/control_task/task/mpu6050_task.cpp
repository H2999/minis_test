// #include "FreeRTOS.h"
// #include "task.h"
// #include "ekf.h"
// #include "FreeRTOSConfig.h"
// #include "MPU6050.h"
// #include "usart.h"
//
// EKF* ekf = nullptr;
// MPU6050& mpu6050 = MPU6050::getinstance();
// uint8_t rx_data;
//
// float dt = 0.001f;
//
// struct frame
// {
//     float data[3];
//     uint8_t tail[4];
// }tx_data;
//
// uint8_t whoami = 0x00;
//
// float yaw,pitch,roll{};
//
// extern "C"
// {
//     void get_data()
//     {
//         read_scoped_lock mpu6050_lock(MPU6050::get_lock());
//         MPU6050::getinstance().MPU6050_ReadRaw();
//         MPU6050::getinstance().MPU6050_ReadData();
//
//         ekf->EKF_Prediction(mpu6050.get_data().gyro_final[0],mpu6050.get_data().gyro_final[1],
//                             mpu6050.get_data().gyro_final[2]);
//         ekf->EKF_Update(mpu6050.get_data().accel_final[0],mpu6050.get_data().accel_final[1],
//                             mpu6050.get_data().accel_final[2]);
//
//         float q[4] = {0.0f};
//         q[0] = ekf->get_data().x[0];
//         q[1] = ekf->get_data().x[1];
//         q[2] = ekf->get_data().x[2];
//         q[3] = ekf->get_data().x[3];
//
//         ekf->QuaternionToEuler(q,&yaw,&roll,&pitch);
//
//         tx_data.data[0] = yaw;
//         tx_data.data[1] = roll;
//         tx_data.data[2] = pitch;
//
//         HAL_UART_Transmit_DMA(&huart2,reinterpret_cast<uint8_t *>(&tx_data),sizeof(tx_data));
//     }
//
//     void mpu6050_thread(void * argument)
//     {
//         MPU6050::getinstance().MPU6050_ReadReg(0x75, &whoami);
//
//         tx_data.tail[0] = 0x00;
//         tx_data.tail[1] = 0x00;
//         tx_data.tail[2] = 0x80;
//         tx_data.tail[3] = 0x7f;
//         HAL_UART_Receive_IT(&huart2,&rx_data,sizeof(rx_data));
//
//         while (1)
//         {
//             // get_data();
//             vTaskDelay(5);
//         }
//     }
//
//     void start_mpu6050_task(void *argument)
//     {
//         ekf = new EKF();
//         if (ekf->EKF_Init(dt) != HAL_OK)
//         {
//             return;
//         }
//
//         xTaskCreate(mpu6050_thread,"mpu6050_thread",256,nullptr,
//                 configMAX_PRIORITIES - 3, nullptr);
//
//         vTaskDelete(nullptr);
//     }
//
//     void HAL_GPIO_EXTI_Callback(uint16_t GPIO_Pin)
//     {
//         if (GPIO_Pin == GPIO_PIN_5)
//         {
//             tx_data.data[0] = mpu6050.get_data().temperature;
//             tx_data.data[1] = 0.0f;
//             tx_data.data[2] = 0.0f;
//         }
//         if (GPIO_Pin == GPIO_PIN_15)
//         {
//             tx_data.data[0] = whoami;
//             tx_data.data[1] = 0.0f;
//             tx_data.data[2] = 0.0f;
//         }
//         HAL_UART_Transmit_DMA(&huart2,(uint8_t*)&tx_data,sizeof(tx_data));
//     }
//
//     void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart)
//     {
//         if (huart == &huart2)
//         {
//             if (rx_data == 'G')
//             {
//                 tx_data.data[0] = mpu6050.get_data().gyro_raw[0];
//                 tx_data.data[1] = mpu6050.get_data().gyro_raw[1];
//                 tx_data.data[2] = mpu6050.get_data().gyro_raw[2];
//             }
//             if (rx_data == 'A')
//             {
//                 tx_data.data[0] = mpu6050.get_data().accel_raw[0];
//                 tx_data.data[1] = mpu6050.get_data().accel_raw[1];
//                 tx_data.data[2] = mpu6050.get_data().accel_raw[2];
//             }
//             if (rx_data == 'T')
//             {
//                 tx_data.data[0] = mpu6050.get_data().temperature;
//                 tx_data.data[1] = 0.0f;
//                 tx_data.data[2] = 0.0f;
//             }
//             HAL_UART_Transmit_DMA(&huart2,(uint8_t*)&tx_data,sizeof(tx_data));
//         }
//         HAL_UART_Receive_IT(&huart2,&rx_data,sizeof(rx_data));
//     }
// }
