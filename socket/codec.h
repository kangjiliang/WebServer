#ifndef _CODEC_H
#define _CODEC_H

#include <iostream>
#include <string>

using namespace std;

string url_encode(const string& src);
string url_decode(const string& src);

string base64_encode(const string& src);
string base64_decode(const string& src);

char * sha1(const char *msg, size_t size, char *result);

#endif
