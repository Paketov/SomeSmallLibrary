
#ifndef __EX_BASE64_h__HAS_INCLUDED__
#define __EX_BASE64_h__HAS_INCLUDED__

#include <string>

size_t CodeBase64(char *Dest, const void* Src, size_t SrcLen);
size_t CodeBase64URL(char *Dest, const void* Src, size_t SrcLen);


std::basic_string<char> CodeBase64(const void *Src, size_t SrcLen);
std::basic_string<char> CodeBase64URL(const void *Src, size_t SrcLen);

int DecodeBase64(void *Dst, const char *Src, size_t SrcLen);
int DecodeBase64URL(void *Dst, const char *Src, size_t SrcLen);

std::basic_string<char> DecodeBase64(const char *Src, size_t SrcLen);
std::basic_string<char> DecodeBase64URL(const char *Src, size_t SrcLen);

#endif


