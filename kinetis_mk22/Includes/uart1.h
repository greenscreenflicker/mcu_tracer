/*
 * uart1.h
 *
 *  Created on: 02.05.2016
 *      Author: Michael
 */

#ifndef PROJECT_HEADERS_UART1_H_
#define PROJECT_HEADERS_UART1_H_


void UART1_init_defaultparam();
void UART1_startup(int baudrate);
void UART1_txChar(int ch);
void UART1_txBulk(uint8_t *data, uint16_t len);
char UART1_getch(void);
void UART1_DMA_CONFIG(void);
void dma_idiotentest(void);
uint16_t UART1_rec_size(void);
void UART1_dma_complete_wait(void);
void UART1_dma_config_rec(void);
void UART1_dmaconfig(void);
char UART1_buffercontent(void);

#endif /* PROJECT_HEADERS_UART1_H_ */
