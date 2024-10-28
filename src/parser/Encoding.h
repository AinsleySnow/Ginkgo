#ifndef _ENCODING_H_
#define _ENCODING_H_

#include <string>


enum class Encoding {
  NONE,
  CHAR16,
  CHAR32,
  UTF8,
  WCHAR
};


void ConvertToUTF16(std::string& str);
void ConvertToUTF32(std::string& str);
void AppendUCN(std::string& str, int c);

#endif // _ENCODING_H_
