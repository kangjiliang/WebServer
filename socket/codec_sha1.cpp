#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <assert.h>


/*================================================================================================
SHA1的算法描述：
输入一个消息(可以是字符串，也可以是二进制码流)
输出160位，也就是20字节
SHA1始终把消息当成一个位串来处理，每次将512位当做一个数据块来处理，算法分为下面5个步骤
1. 补位
    使长度对512取模后余448，剩下64位填充原始数据的长度
    先补一个1，然后再补0
    即使原本已满足对512取模后余448，也要补位
2. 剩下的64位填充消息的原始长度
3. 按512位数据块依次处理，每个数据块64字节
    1> 64个字节，分成16个uint32的数据 M1,M2,M3...M16
    2> 16个uint32的数据扩充到80个，<<< 表示循环左移
       words[t] = (words[t - 3] ^ words[t - 8] ^ words[t - 14] ^ words[t - 16]) <<< 1
    3> SHA1最终结果是20字节，也就是5个uint32，5个的初值分别为：
       A=0x67452301
       B=0xEFCDAB89
       C=0x98BADCFE
       D=0x10325476
       E=0xC3D2E1F0
    4> 对前面产生的80个uint32的数据，进行处理：
        1) 处理前，先保存A,B,C,D,E的初值
        2) 按下面的运算处理80次
           A,B,C,D,E←[(A<<<5)+ ft(B,C,D)+E+Wt+Kt],A,(B<<<30),C,D
           将[(A<<<5)+ ft(B,C,D)+E+Wt+Kt] 的结果赋值给A
           A的初始值 赋值给B
           B的初始值循环左移30位 赋值给C
           C的初始值赋值给D
           D的初始值赋值给E
        3) 其中
            0≤t≤19 时 ft(B,C,D)=(B·C)V(~B·D)        Kt=5A827999
           20≤t≤39 时 ft(B,C,D)=B⊕C⊕D               Kt=6ED9EBA1
           40≤t≤59 时 ft(B,C,D)=(B·C)V(B·D)V(C·D)， Kt=8F188CDC
           60≤t≤79 时 ft(B,C,D)=B⊕C⊕D               Kt=CA62C1D6
    5>80次计算的结果 加上A B C D E的初始值，作为下一个数据块的初始值
*/


//SHA1的结果数据长度
#define SHA1_RESULT_SIZE  20
//每次处理的BLOCK的大小
#define SHA1_BLOCK_SIZE   64
//反转字节序
#define SHA1_SWAP16(x)  ((((x) & 0xff00) >>  8) | (((x) & 0x00ff) <<  8))
#define SHA1_SWAP32(x)  ((((x) & 0xff000000) >> 24) | \
                         (((x) & 0x00ff0000) >>  8) | \
                         (((x) & 0x0000ff00) <<  8) | \
                         (((x) & 0x000000ff) << 24))

#define SHA1_SWAP64(x)  ((((x) & 0xff00000000000000) >> 56) | (((x) & 0x00ff000000000000) >> 40) | \
                         (((x) & 0x0000ff0000000000) >> 24) | (((x) & 0x000000ff00000000) >>  8) | \
                         (((x) & 0x00000000ff000000) <<  8) | (((x) & 0x0000000000ff0000) << 24) | \
                         (((x) & 0x000000000000ff00) << 40) | (((x) & 0x00000000000000ff) << 56))
//循环左右位移
#define ROTL32(dword, n) ((dword) << (n) ^ ((dword) >> (32 - (n))))
#define ROTR32(dword, n) ((dword) >> (n) ^ ((dword) << (32 - (n))))
#define ROTL64(qword, n) ((qword) << (n) ^ ((qword) >> (64 - (n))))
#define ROTR64(qword, n) ((qword) >> (n) ^ ((qword) << (64 - (n))))

//SHA1算法的上下文，保存一些状态，中间数据，结果
struct sha1_context
{
    //报文总长度
    uint64_t msglen;
    //还没有处理的数据长度
    uint64_t unproclen;
    //数据块缓冲区
    uint32_t block[16];
    //SHA1算法缓冲区
    uint32_t hash[5];
    //SHA1算法每个数据块 4轮处理的K值
    uint32_t kval[4];
};


//判断当前主机为小端字节序
static uint8_t sha1_isle(){
    uint16_t flag = 1;
    return (*(uint8_t*)&flag) ? 1 : 0;
}

//SHA1算法的上下文的初始化
static void sha1_init(sha1_context *ctx)
{
    ctx->msglen = 0;
    ctx->unproclen = 0;

    ctx->hash[0] = 0x67452301;
    ctx->hash[1] = 0xefcdab89;
    ctx->hash[2] = 0x98badcfe;
    ctx->hash[3] = 0x10325476;
    ctx->hash[4] = 0xc3d2e1f0;

    ctx->kval[0] = 0x5A827999;
    ctx->kval[1] = 0x6ED9EBA1;
    ctx->kval[2] = 0x8F1BBCDC;
    ctx->kval[3] = 0xCA62C1D6;
}


// A,B,C,D,E←[(A<<<5)+ ft(B,C,D)+E+Wt+Kt],A,(B<<<30),C,D
// ft(B,C,D)=(B·C)V(~B·D)      0≤t≤19
// ft(B,C,D)=B⊕C⊕D           20≤t≤39
// ft(B,C,D)=(B·C)V(B·D)V(C·D) 40≤t≤59
// ft(B,C,D)=B⊕C⊕D           60≤t≤79
static void sha1_process_block_hash_value(sha1_context *ctx, uint32_t hash[5], uint32_t wblock[80], uint32_t t)
{
    uint32_t ft = 0;
    uint32_t step = t / 20;
    uint32_t temp = 0;
    switch(step)
    {
        case 0:
            ft = (hash[1] & hash[2]) | (~hash[1] & hash[3]);
            break;
        case 1:
            ft = (hash[1] ^ hash[2] ^ hash[3]);
            break;
        case 2:
            ft = ((hash[1] & hash[2]) | (hash[1] & hash[3]) | (hash[2] & hash[3]));
            break;
        case 3:
            ft = (hash[1] ^ hash[2] ^ hash[3]);
            break;
        default:
            break;
    }
    temp = ROTL32(hash[0], 5) + ft + hash[4] + wblock[t] + ctx->kval[step];
    hash[4] = hash[3];
    hash[3] = hash[2];
    hash[2] = ROTL32(hash[1], 30);
    hash[1] = hash[0];
    hash[0] = temp;
}

//处理每一个512位的数据块
static void sha1_process_block(sha1_context *ctx)
{
    uint32_t t;
    uint32_t words[80];
    uint32_t thash[5];
    //先扩展到80个word
    for(t = 0; t < 16; t++)
    {
        words[t] = sha1_isle() ? SHA1_SWAP32(ctx->block[t]) : ctx->block[t];
    }

    for (t = 16; t < 80; t++)
    {
        words[t] = ROTL32(words[t - 3] ^ words[t - 8] ^ words[t - 14] ^ words[t - 16], 1);
    }
    //保存初始值
    memcpy(thash, ctx->hash, sizeof(ctx->hash));

    for(t = 0; t < 80; t++)
    {
        sha1_process_block_hash_value(ctx, thash, words, t);
    }
    //初始值加上80次计算的结果 作为下一个数据块的初始值
    ctx->hash[0] += thash[0];
    ctx->hash[1] += thash[1];
    ctx->hash[2] += thash[2];
    ctx->hash[3] += thash[3];
    ctx->hash[4] += thash[4];
}

static void sha1_update(sha1_context *ctx, const char *buf, size_t size)
{
    //sha1_update可多次进入
    ctx->msglen += size;

    //每个处理的块都是64字节
    while (size >= SHA1_BLOCK_SIZE)
    {
        memcpy(ctx->block, buf, SHA1_BLOCK_SIZE);
        sha1_process_block(ctx);
        buf  += SHA1_BLOCK_SIZE;
        size -= SHA1_BLOCK_SIZE;
    }

    ctx->unproclen = size;
}


static void sha1_padzero(unsigned char block[SHA1_BLOCK_SIZE], uint32_t index, uint32_t max)
{
    while (index < max)
    {
        block[index++] = 0;
    }
}


static void sha1_final(sha1_context *ctx, const char *msg, size_t size, char *result)
{
    unsigned char block[SHA1_BLOCK_SIZE];

    memset(block, 0, sizeof(block));
    if(ctx->unproclen)
    {
        memcpy(block, msg + size - ctx->unproclen, ctx->unproclen);
    }
    uint32_t index = ctx->unproclen;
    //先补一位0
    block[index++] = 0x80;
    //如果大于56 剩下的不够8个字节填长度 则补0 再计算一次数据块
    if(index > 56)
    {
        sha1_padzero(block, index, 64);
        memcpy(ctx->block, block, SHA1_BLOCK_SIZE);
        sha1_process_block(ctx);
        index = 0;
    }
    //不够56 则补0到56字节
    sha1_padzero(block, index, 56);
    //填报文的bit位的长度
    uint64_t bitlen = ctx->msglen << 3;
    bitlen = sha1_isle() ? SHA1_SWAP64(bitlen) : bitlen;
    memcpy(block+56, &bitlen, sizeof(bitlen));
    memcpy(ctx->block, block, SHA1_BLOCK_SIZE);
    sha1_process_block(ctx);

    //结果是大端字节序 如果本机是小端 则需要转换
    if(sha1_isle())
    {
        ctx->hash[0] = SHA1_SWAP32(ctx->hash[0]);
        ctx->hash[1] = SHA1_SWAP32(ctx->hash[1]);
        ctx->hash[2] = SHA1_SWAP32(ctx->hash[2]);
        ctx->hash[3] = SHA1_SWAP32(ctx->hash[3]);
        ctx->hash[4] = SHA1_SWAP32(ctx->hash[4]);
    }
    memcpy(result, &ctx->hash, sizeof(ctx->hash));
}



//计算一个内存数据的SHA1值
char * sha1(const char *msg, size_t size, char *result)
{
    sha1_context ctx;
    sha1_init(&ctx);
    sha1_update(&ctx, msg, size);
    sha1_final(&ctx, msg, size, result);
    return result;
}

#ifdef _TEST_CODEC_URL
int main()
{
    int ret = 0;
    static char test_buf[7][81] =
    {
        { "" },
        { "a" },
        { "abc" },
        { "message digest" },
        { "abcdefghijklmnopqrstuvwxyz" },
        { "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789" },
        { "12345678901234567890123456789012345678901234567890123456789012345678901234567890" }
    };

    static const size_t test_buflen[7] =
    {
        0, 1, 3, 14, 26, 62, 80
    };

    char result[32] ={0};


    static const unsigned char sha1_test_sum[7][20] =
    {
        { 0xda,0x39,0xa3,0xee,0x5e,0x6b,0x4b,0x0d,0x32,0x55,0xbf,0xef,0x95,0x60,0x18,0x90,0xaf,0xd8,0x07,0x09 },
        { 0x86,0xf7,0xe4,0x37,0xfa,0xa5,0xa7,0xfc,0xe1,0x5d,0x1d,0xdc,0xb9,0xea,0xea,0xea,0x37,0x76,0x67,0xb8 },
        { 0xa9,0x99,0x3e,0x36,0x47,0x06,0x81,0x6a,0xba,0x3e,0x25,0x71,0x78,0x50,0xc2,0x6c,0x9c,0xd0,0xd8,0x9d },
        { 0xc1,0x22,0x52,0xce,0xda,0x8b,0xe8,0x99,0x4d,0x5f,0xa0,0x29,0x0a,0x47,0x23,0x1c,0x1d,0x16,0xaa,0xe3 },
        { 0x32,0xd1,0x0c,0x7b,0x8c,0xf9,0x65,0x70,0xca,0x04,0xce,0x37,0xf2,0xa1,0x9d,0x84,0x24,0x0d,0x3a,0x89 },
        { 0x76,0x1c,0x45,0x7b,0xf7,0x3b,0x14,0xd2,0x7e,0x9e,0x92,0x65,0xc4,0x6f,0x4b,0x4d,0xda,0x11,0xf9,0x40 },
        { 0x50,0xab,0xf5,0x70,0x6a,0x15,0x09,0x90,0xa0,0x8b,0x2c,0x5e,0xa4,0x0f,0xa0,0xe5,0x85,0x55,0x47,0x32 },
    };
    for(size_t i=0;i<7;++i)
    {
        sha1(test_buf[i],test_buflen[i],result);
        ret = memcmp(result,sha1_test_sum[i],20);
        if (ret != 0)
        {
            assert(false);
        }
    }
}
#endif


