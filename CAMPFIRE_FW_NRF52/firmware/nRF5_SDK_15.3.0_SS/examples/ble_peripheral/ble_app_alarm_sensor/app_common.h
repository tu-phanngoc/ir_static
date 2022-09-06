#ifndef __APP_COMMON__
#define __APP_COMMON__
#include <stdint.h>
#include <stdbool.h>

/*
 * Ring buffer
 */
#define SER_BUF_SIZE 	(1024)
#define SER_BUF_MASK	(SER_BUF_SIZE - 1ul)

#define SER_BUF_RESET(serBuf)		(serBuf.rdIdx = serBuf.wrIdx = 0)
#define SER_BUF_WR(serBuf, dataIn)	(serBuf.data[SER_BUF_MASK & serBuf.wrIdx++] = (dataIn))
#define SER_BUF_RD(serBuf)			(serBuf.data[SER_BUF_MASK & serBuf.rdIdx++])
#define SER_BUF_EMPTY(serBuf)		(serBuf.rdIdx == serBuf.wrIdx)
#define SER_BUF_FULL(serBuf)		(serBuf.wrIdx == ((serBuf.rdIdx + SER_BUF_SIZE) & 0x0FFFF))
#define SER_BUF_RQ_WR(serBuf, dataIn)	(serBuf.data[SER_BUF_MASK & serBuf.wrIdx] = (dataIn))

typedef struct __SER_BUF_T {
	uint16_t data[SER_BUF_SIZE];
	unsigned short wrIdx;
	unsigned short rdIdx;
} SER_BUF_T;


#endif
