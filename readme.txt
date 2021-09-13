https://www.youtube.com/watch?v=LNpbvyLIvN0

www.bigclive.com/buddha.bin


/* **** */


seems to be several distinct blocks of data...

0x000000 -- 0x0005ff
0x000600 -- 0x007dff
0x007e00 -- 0x008dff
0x008e00 -- 0x0096ff
0x009700 -- 0x200000


/* **** */

looking at what seems to be the header...

0xxxxxxxxx: ef ff zz zz 1f zz zz zz f0 zz zz zz ef ff df zz 50 zz zz de 86 d0 82 e7 10 00 20 41 83 07 0f e3

many entries seem to follow a distinct format...

where valid hex bytes remain consistent...  zz are bytes that consistently vary.

/* **** */

suggested offset at 4... with xor 0x1F3E7CF8
suggested length at 8... with xor 0xF0C1A367

/* **** */

0x00000000: 17 94 04 e3 1f 21 77 e8 2b bd e7 35 ef ff df b4 c1 83 07 0f 1f 1e 1c 18 bc b7 ea 8e 32 f8 0f 3e 
0x00000020: ef ff 18 a3 1f 3e 7a f8 f0 c1 db 67 ef ff df 9f 5d 13 9c 95 ce 80 93 97 ef 00 20 41 83 07 0f 3e 
0x00000040: ef ff 57 8b 1f 3e 02 f8 f0 c1 b3 67 ef ff df 9e 4e 10 99 89 bf 8d 8a 94 9b d1 bd d7 12 f8 0f 3e 
0x00000060: ef ff 66 85 1f 3e f2 f8 f0 c1 aa 67 ef ff df 9d 57 18 a7 9c 89 92 97 c9 8d 96 b1 be 83 07 0f 3e 
0x00000080: ef ff a1 f2 1f 3e eb f8 f0 c4 2c 23 ef ff df 9c 50 4c c9 de 86 d0 82 e7 10 00 20 41 83 07 0f 3e 
0x000000a0: ef ff 14 4a 1f 38 5a bc f0 c1 50 67 ef ff df 9b 50 4c ca de 86 d0 82 e7 10 00 20 41 83 07 0f 3e 
0x000000c0: ef ff 3e 98 1f 39 65 bc f0 c1 6f 1d ef ff df 9a 50 4c cb de 86 d0 82 e7 10 00 20 41 83 07 0f 3e 
0x000000e0: ef ff 06 8a 1f 39 99 46 f0 c0 a7 b0 ef ff df 99 50 4c cc de 86 d0 82 e7 10 00 20 41 83 07 0f 3e 
