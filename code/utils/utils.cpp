/*=====================================================================================

    Filename:     utils.cpp

    Description:
        Version:  1.0

    Author:       Peng Lin, penglincs@outlook.com

    Organization: Shaowei Cai Group,
                  State Key Laboratory of Computer Science,
                  Institute of Software, Chinese Academy of Sciences,
                  Beijing, China

=====================================================================================*/

#include "header.h"

std::chrono::_V2::system_clock::time_point TimeNow()
{
  return std::chrono::high_resolution_clock::now();
}

double ElapsedTime(
    const std::chrono::_V2::system_clock::time_point &a,
    const std::chrono::_V2::system_clock::time_point &b)
{
  return std::chrono::duration_cast<std::chrono::milliseconds>(a - b).count() / 1000.0;
}

bool IsBlank(
    const std::string &a)
{
  for (auto x : a)
    if (x != ' ' && x != '\n' && x != '\r')
      return false;
  return true;
}

void PrintfError(
    const std::string &a)
{
  printf("c error line: %s\n", a.c_str());
  exit(-1);
}
