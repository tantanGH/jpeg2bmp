# JPEG2BMP.X

JPEG to BMP converter for X680x0/Human68k

JPEGファイルをBMPファイルに変換します。24bitカラーのベースラインJPEGのみサポートしています。

JPEGデコードライブラリとして Rich Geldreich氏の picojpeg (public domain license) を利用させて頂いています。

---

### インストール方法

JPGBMxxx.ZIP をダウンロードして展開し、JPEG2BMP.X をパスの通ったディレクトリに置きます。

---

### 使用方法

    jpeg2bmp [オプション] <JPEGファイル名>

      -o <出力ファイル名> ... 省略した場合はJPEGファイルの拡張子を.bmpに変えたものになります
      -h    ... ヘルプメッセージを表示します

060loadhigh.x を使ったハイメモリ上での実行に対応しています。

---

### Special Thanks

* xdev68k thanks to ファミべのよっしんさん
* HAS060.X on run68mac thanks to YuNKさん / M.Kamadaさん / GOROmanさん
* HLK301.X on run68mac thanks to SALTさん / GOROmanさん

---

### History

* 0.1.0 (2023/03/24) ... 初版
