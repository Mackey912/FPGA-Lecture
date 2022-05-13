# FPGA-Lecture

## FPGAセットアップ

### ソフトウェア
・Vitis_hls  
・Vivado  
・http://www.pynq.io/board.html にてUltra96v2のv2.7のOS  
・balena etcher  
・Tera Tarm
### ハードウェア
・Ultra96v2 FPGAボード  
・micro SDカード16G以上  
・micro USBケーブル  
・Ultra96v2用ACアダプタ  
・LAN to USB Type A 変換ケーブル (あれば)  
###  セットアップ手順
・balena etcherでFPGA用osをSDカードにフォーマット
・micro USB, LAN, ACアダプタをFPGAに接続
・FPGAのsw4を押す
・Tera Term 等でシリアル通信開始
・ifconfigで確認したip addressにアクセス

## 大まかな流れ
Step1: vitis_hlsにてcppでコード記述、ip化  
Step2: vivadoにてStep1で作成したipとプロセッサとの接続, bitstream化  
Step3: Step2にて作成した.bit, .hwhファイルをFPGAに転送, pynq os上のpythonでip操作  
