# GR-Boards_Microphone_sample
GR-PEACH、および、GR-LYCHEEで動作するサンプルプログラムです。  
GR-LYCHEEの開発環境については、[GR-LYCHEE用オフライン開発環境の手順](https://developer.mbed.org/users/dkato/notebook/offline-development-lychee-langja/)を参照ください。


## 概要
マイク入力をスピーカーから出力するサンプルです。  


### 補足：キャッシュメモリについて
Audioデータの転送にはDMAを使用するため、Audio用バッファは非キャッシュに配置しています。  
キャッシュ制御についての詳細は[こちら](https://github.com/d-kato/mbed-gr-libs)を参照ください。  
```cpp
//32 bytes aligned! No cache memory
#if defined(__ICCARM__)
#pragma data_alignment=32
static uint8_t audio_read_buff[READ_BUFF_NUM][READ_BUFF_SIZE]@ ".mirrorram";
#else
static uint8_t audio_read_buff[READ_BUFF_NUM][READ_BUFF_SIZE] __attribute((section("NC_BSS"),aligned(32)));
#endif
```
