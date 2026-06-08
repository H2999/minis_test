// #include "uart_drv.h"
//
// uart_drv::uart_drv(UART_HandleTypeDef *huart):rx_index(0),_huart(huart)
// {
//     uart_map()[huart] = this;
//     _callback.reserve(10);
//
//     memset(rx_buffer[0], 0, sizeof(rx_buffer[0]));
//     memset(rx_buffer[1], 0, sizeof(rx_buffer[1]));
//
//     HAL_UARTEx_ReceiveToIdle_DMA(_huart, rx_buffer[0], sizeof(rx_buffer[0]));
// }
//
//
// uart_drv::~uart_drv()
// {
//     _callback.clear();
//     uart_map().erase(_huart);
// }
//
// uart_drv* uart_drv::getinstance(UART_HandleTypeDef* huart)
// {
//     const auto drv = uart_map().find(huart);
//     if (drv == uart_map().end())
//     {
//         const auto new_drv = new uart_drv(huart);
//         uart_map()[huart] = new_drv;
//         return new_drv;
//     }
//     return drv->second;
// }
//
// void uart_drv::register_callback(const uart_callback_t cb)
// {
//     if (_callback.size() == _callback.capacity())
//     {
//         return;
//     }
//     _callback.push_back(cb);
// }
//
// std::map<UART_HandleTypeDef*,uart_drv*> &uart_drv::uart_map()
// {
//     static std::map<UART_HandleTypeDef*,uart_drv*> uart_map;
//     return uart_map;
// }
//
// void uart_drv::enable_dma()
// {
//     // rx_index ^= 0x01;
//     HAL_UARTEx_ReceiveToIdle_DMA(_huart,rx_buffer[rx_index],sizeof(rx_buffer[rx_index]));
// }
//
// // extern "C"
// // {
// //     void HAL_UARTEx_RxEventCallback(UART_HandleTypeDef *huart, uint16_t Size)
// //     {
// //         const auto it = uart_drv::uart_map().find(huart);
// //         static BaseType_t xHigherPriorityTaskWoken = pdFALSE;
// //         if (it != uart_drv::uart_map().end() && it->second)
// //         {
// //             auto drv = it->second;
// //             for (auto &cb : drv->_callback)
// //             {
// //                 if (cb(drv->rx_buffer[drv->rx_index],Size,xHigherPriorityTaskWoken))
// //                 {
// //                     drv->rx_index ^= 0x01U;
// //                     break;
// //                 }
// //             }
// //             drv->enable_dma();
// //         }
// //         if (xHigherPriorityTaskWoken)
// //         {
// //             portYIELD_FROM_ISR(xHigherPriorityTaskWoken);
// //         }
// //     }
// // }