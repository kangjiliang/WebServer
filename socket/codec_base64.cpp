#include "codec.h"

/*
base64编码 3个字节一组 24位 拆分成4组 每组6位 用4个数字去ENMAP中索引
不够3的倍数时，末尾补1或2个 =
编码长度增加1/3


base64解码 和编码流程相反

*/

const static char BASE64ENPAD = '=';
const static char BASE64ENMAP[64] =   
{  
    'A', 'B', 'C', 'D', 'E', 'F', 'G', 'H', 'I', 'J',
    'K', 'L', 'M', 'N', 'O', 'P', 'Q', 'R', 'S', 'T', 
    'U', 'V', 'W', 'X', 'Y', 'Z', 'a', 'b', 'c', 'd', 
    'e', 'f', 'g', 'h', 'i', 'j', 'k', 'l', 'm', 'n', 
    'o', 'p', 'q', 'r', 's', 't', 'u', 'v', 'w', 'x', 
    'y', 'z', '0', '1', '2', '3', '4', '5', '6', '7', 
    '8', '9', '+', '/'
};  


const static char BASE64DEMAP[128] =   
{  
    0,   0,  0,  0,  0,  0,  0,  0,  0,  0,   
    0,   0,  0,  0,  0,  0,  0,  0,  0,  0,   
    0,   0,  0,  0,  0,  0,  0,  0,  0,  0,   
    0,   0,  0,  0,  0,  0,  0,  0,  0,  0,   
    0,   0,  0, 62,  0,  0,  0, 63, 52, 53,   
    54, 55, 56, 57, 58, 59, 60, 61,  0,  0,   
    0,  61,  0,  0,  0,  0,  1,  2,  3,  4,   
    5,   6,  7,  8,  9, 10, 11, 12, 13, 14,   
    15, 16, 17, 18, 19, 20, 21, 22, 23, 24,   
    25,  0,  0,  0,  0,  0,  0, 26, 27, 28,   
    29, 30, 31, 32, 33, 34, 35, 36, 37, 38,   
    39, 40, 41, 42, 43, 44, 45, 46, 47, 48,   
    49, 50, 51,  0,  0,  0,  0,  0
};


static unsigned char encode0(char c0)
{
    return ((c0 >> 2) & 0x3F);
}

static unsigned char encode1(char c0, char c1)
{
    return ((c0 & 0x03) << 4) | ((c1 & 0xF0) >> 4);
}

static unsigned char encode2(char c1, char c2)
{
    return ((c1 & 0x0F) << 2) | ((c2 & 0xC0) >> 6);
}

static unsigned char encode3(char c2)
{
    return (c2 & 0x3F);
}

static char decode0(char c0, char c1)
{
    return (c0 << 2) | (c1 >> 4);
}

static char decode1(char c1, char c2)
{
    return (c1 << 4) | (c2 >> 2);
}

static char decode2(char c2, char c3)
{
    return (c2 << 6) | (c3);
}



string base64_encode(const string& src)
{
    int m = 0;
    int n = 0;
    string dst;
    char c0, c1, c2;

    m = src.size() / 3;
    n = src.size() % 3;

    for(int i = 0; i < m; i++)
    {
        c0 = src[i*3 + 0];
        c1 = src[i*3 + 1];
        c2 = src[i*3 + 2];
        dst.push_back(BASE64ENMAP[encode0(c0)]);
        dst.push_back(BASE64ENMAP[encode1(c0, c1)]);
        dst.push_back(BASE64ENMAP[encode2(c1, c2)]);
        dst.push_back(BASE64ENMAP[encode3(c2)]);
    }
    switch(n)
    {
        case 1:
            c0 = src[m*3 + 0];
            dst.push_back(BASE64ENMAP[encode0(c0)]);
            dst.push_back(BASE64ENMAP[encode1(c0, 0)]);
            dst.push_back(BASE64ENPAD);
            dst.push_back(BASE64ENPAD);
            break;
        case 2:
            c0 = src[m*3 + 0];
            c1 = src[m*3 + 1];
            dst.push_back(BASE64ENMAP[encode0(c0)]);
            dst.push_back(BASE64ENMAP[encode1(c0, c1)]);
            dst.push_back(BASE64ENMAP[encode2(c1, 0)]);
            dst.push_back(BASE64ENPAD);        
            break;
        case 0:
        default:
            break;
    }
    return dst;
}

string base64_decode(const string& src)
{
    string dst;
    char c0, c1, c2, c3;
    int  size = src.size();
    for(int i = 0; i < size; i+=4)
    {
        c0 = BASE64DEMAP[(unsigned char)src[i + 0]];
        c1 = BASE64DEMAP[(unsigned char)src[i + 1]];
        c2 = BASE64DEMAP[(unsigned char)src[i + 2]];
        c3 = BASE64DEMAP[(unsigned char)src[i + 3]];
        dst.push_back(decode0(c0, c1));
        if('=' == c2)
        {
            break;
        }
        dst.push_back(decode1(c1, c2));
        if('=' == c3)
        {
            break;
        }
        dst.push_back(decode2(c2, c3));
    }
    return dst;
}

#ifdef _TEST_CODEC_BASE64
int main()
{
    string src = "BASE64加密解密";
    string dst = base64_encode(src);

    cout << src << endl;
    cout << dst << endl;
    cout << base64_decode(dst) << endl;
}
#endif