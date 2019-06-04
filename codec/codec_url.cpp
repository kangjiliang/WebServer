
#include "codec.h"

/*
字符 'a'-'z', 'A'-'Z', '0'-'9', '.', '-', '_', '*' 保持原值
空格转为 '+'
其他每个字节转为 %xy  
*/


static const string URLHEXMAP = "0123456789ABCDEF";

string url_encode(const string& src)
{
    string dst;
    string::size_type size  = src.size();
    string::size_type index = 0;
    char c = 0;

    for(index = 0; index < size; index++)
    {
        c = src[index];
        if(('-' == c) || ('_' == c) || ('*' == c) || ('.' == c) ||
          (('0' <= c) && ('9' >= c))||
          (('a' <= c) && ('z' >= c))||
          (('A' <= c) && ('Z' >= c)))
        {
            dst.push_back(c);
        }
        else if(' ' == c)
        {
            dst.push_back('+');
        }
        else
        {
            dst.push_back('%');
            dst.push_back(URLHEXMAP[(c & 0xF0) >> 4]);
            dst.push_back(URLHEXMAP[(c & 0x0F)]);
        }
    }
    return dst;
}

string url_decode(const string& src)
{
    string dst;
    string::size_type size  = src.size();
    string::size_type index = 0;
    unsigned char  index_c1 = 0;
    unsigned char  index_c2 = 0;

    for(index = 0; index < size; index++)
    {
        if('%' == src[index] && (index+2) < size && isxdigit(src[index+1]) && isxdigit(src[index+2]))
        {
            index_c1 = URLHEXMAP.find(src[++index]);
            index_c2 = URLHEXMAP.find(src[++index]);
            dst.push_back(index_c1 << 4 | index_c2);
        }
        else if('+' == src[index])
        {
            dst.push_back(' ');
        }
        else
        {
            dst.push_back(src[index]);
        }
    }
    return dst;
}

#ifdef _TEST_CODEC_URL
int main()
{
    string src = "测试URL编码 其他每个字节转为 %xy";
    string dst = url_encode(src);

    cout << src << endl;
    cout << dst << endl;
    cout << url_decode(dst) << endl;
    cout << url_encode("二十分公司答复")<< endl;
    cout << url_decode("%E4%BA%8C%E5%8D%81%E5%88%86%E5%85%AC%E5%8F%B8%E7%AD%94%E5%A4%8D") << endl;
}
#endif
