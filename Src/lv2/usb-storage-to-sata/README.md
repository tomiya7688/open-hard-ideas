# USB Mass Storage → SATA Device Bridge

**Level:** Lv2  
**Issue:** https://github.com/tomiya7688/open-hard-ideas/issues/1

USBメモリなどの USB Mass Storage デバイスを、PC側から通常の SATA HDD/SSD として扱えるようにするブリッジです。

一般的な「SATAドライブをUSBから使う」変換とは逆方向を目指します。

## Use case

ノートPCなどのSATAポートへ接続し、USBメモリ上のテスト環境をSATAドライブとして見せます。

これにより、SSD/HDDを都度用意せず、USBメモリを書き換えるだけでPCの動作確認や起動試験を行いやすくすることを狙います。

## Target architecture

```text
USB Mass Storage
      ↓
 USB Host MSC
      ↓
 block_device_t
      ↓
 ATA target core
      ↓
 SATA Device transport / FIS
      ↓
 SATA PHY
      ↓
 SATA Host
```

## Why Lv2

仕様の骨格が固まり、組み込み向けCによる ATA command / block bridge の実装が存在するため Lv2 としています。

現在の実装は次を扱います。

- IDENTIFY DEVICE
- READ SECTORS EXT
- READ DMA EXT
- WRITE SECTORS EXT
- WRITE DMA EXT
- FLUSH CACHE EXT
- block backend abstraction
- RAM-backed host test

## Embedded policy

コアは実機へ持ち込みやすいよう、次を前提にしています。

- C11
- heap allocationなし
- OS依存なし
- filesystem依存なし
- thread不要
- block I/Oはcallbackで差し替え
- 512-byte logical sector

Pythonは将来の外部テストベンチに使うことはできますが、製品側コアの必須依存にはしません。

## Current implementation boundary

現時点では、SATA transportから呼ばれるATA command coreまでです。

未実装:

- USB enumeration / USB Mass Storage transport
- SATA FIS encode/decode
- SATA Device transport state machine
- SATA PHY / OOB / SerDes
- NCQ
- SMART
- TRIM / DSM

既存IC、既存OSS、MCU peripheral、FPGA IPで代替できる部分は、原則として再実装しません。

## Build / test

```sh
make test
```

期待結果:

```text
PASS: ATA target block bridge reference tests
```

このテストはATA command core単体のテストであり、Lv3のSATA/USBシミュレーション完了を意味しません。

## Lv3 requirements

Lv3へ昇格するには、少なくとも以下をOSS環境で再現可能にします。

1. SATA Host側からIDENTIFYを発行し、妥当な応答が返る
2. SATA HostのREADがUSB Mass Storage側の同じLBAへ到達する
3. SATA HostのWRITEがUSB Mass Storage側へ反映される
4. エラーや範囲外アクセスを適切に処理する
5. 第三者が同じシミュレーションを実行してPASS/FAILを確認できる

## Next steps

1. 組み込みUSB Host MSC stack用の `block_device_t` adapterを追加する
2. SATA Device FIS / transportの境界interfaceを定義する
3. SATA Host BFMまたは既存OSSモデルを接続する
4. USB Mass Storage側のモデルまたは実stackとの統合テストを作る
5. 実機構成に使うMCU / SoC / FPGA / PHYを選定する
