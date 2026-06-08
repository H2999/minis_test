// #ifndef UART_DRV_H
// #define UART_DRV_H
//
// #include <cstdint>
// #include <vector>
// #include <map>
// #include <cstring>
// #include "projdefs.h"
//
// #include "portmacro.h"
// #include "stm32f1xx.h"
//
// class uart_drv
// {
// public:
//     using uart_callback_t = bool (*)(uint8_t* p_data,uint8_t Size,BaseType_t& xHigherPriorityTaskWoken);
//
//     ~uart_drv();
//     uart_drv(const uart_drv&) = delete;
//     uart_drv& operator=(const uart_drv&) = delete;
//
//     static uart_drv* getinstance(UART_HandleTypeDef *huart);
//
//     void enable_dma();
//
//     static std::map<UART_HandleTypeDef*,uart_drv*> &uart_map();
//     std::vector<uart_callback_t> _callback;
//
//     uint8_t rx_buffer[2][50]{};
//     uint8_t rx_index;
//     void register_callback(uart_callback_t cb);
// private:
//     explicit uart_drv(UART_HandleTypeDef *huart);
//     UART_HandleTypeDef *_huart;
// };
//
//
// #endif //UART_DRV_H