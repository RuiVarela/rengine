#pragma once

typedef unsigned char Bytef;

#ifdef __cplusplus
extern "C" {
#endif

	void uncompress(Bytef* out, unsigned long* outbytes, Bytef* in, unsigned long inbytes);
	void compress2(Bytef *out, unsigned long* outbytes, Bytef* in, unsigned long inbytes, int level);

#ifdef __cplusplus
}
#endif