#pragma once

#include <vector>

#ifdef __CINT__
#pragma link C++ class vector<unsigned char>+;
#pragma link C++ class vector<unsigned int>+;
#pragma link C++ class vector<vector<float> >+;
#pragma link C++ class vector<unsigned short>+;
#pragma link C++ class vector<unsigned long long>+;
#pragma link C++ class vector<float>+;
#endif

template class vector<unsigned char>;
template class vector<unsigned int>;
template class vector<vector<float> >;
template class vector<unsigned short>;
template class vector<unsigned long long>;
template class vector<float>;
