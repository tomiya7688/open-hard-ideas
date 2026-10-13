# open-hard-ideas

> 「こんなハードウェアがあったらいいな」を Issue に投げ、仕様・設計・シミュレーションまで育てる場所。

このリポジトリは、ハードウェア製品のアイデアを集め、成熟度に応じて **Lv0 → Lv1 → Lv2 → Lv3** と昇格させていくためのリポジトリです。

基本単位は **1 Issue = 1製品案** です。

## Levels

| Level | 状態 | 成果物 |
| --- | --- | --- |
| **Lv0 — Idea / Spec Draft** | アイデアから仕様を固めている途中 | Issue上の議論・要件・用途 |
| **Lv1 — Specified** | 仕様が固まった | 製品README |
| **Lv2 — Design / Implementation** | 設計または実装が存在する | 設計図、回路図、KiCadデータ、C/HDL等のソースのいずれか |
| **Lv3 — Open Simulation Verified** | シミュレーション段階が完了 | 再現可能なOSSシミュレーションとPASS結果 |

Levelは目標ではなく、**現在到達している最高レベル** を表します。

### Lv0 — Idea / Spec Draft

思いついた案はまず Issue にします。

用途、必要なI/O、期待動作、制約、既存製品との差などを議論し、仕様を固めます。

Lv0ではまだ `Src/` に製品ディレクトリを作る必要はありません。

### Lv1 — Specified

何を作るのか、どのように振る舞うのかという仕様が固まった段階です。

この時点で、

```text
Src/lv1/<product-name>/README.md
```

を作成します。

READMEはその製品の仕様書兼入口です。

### Lv2 — Design / Implementation

製品READMEに加えて、**設計図または実装ソースの少なくとも一方** が存在する段階です。

例:

- ブロック図
- 回路図
- KiCad等のEDAデータ
- BOMや主要部品選定
- FPGA / HDL
- MCU firmware
- C / C++等の組み込みコード
- 機構設計データ

Lv2へ昇格したら、製品ディレクトリごと移動します。

```text
Src/lv1/<product-name>/
        ↓
Src/lv2/<product-name>/
```

### Lv3 — Open Simulation Verified

主要な期待動作について、OSSのシミュレーション環境で検証が完了した段階です。

Lv3へ昇格したら、

```text
Src/lv2/<product-name>/
        ↓
Src/lv3/<product-name>/
```

へ移動します。

**Lv3は実機での動作保証を意味しません。**

## Repository layout

`Src` という名前ですが、ここでいう source はソフトウェアソースだけを意味しません。

各製品を再現・検討するための一次成果物をまとめて置きます。

```text
Src/
├── lv1/
│   └── <product-name>/
│       └── README.md
├── lv2/
│   └── <product-name>/
│       ├── README.md
│       ├── schematic/
│       ├── hardware/
│       ├── include/
│       ├── src/
│       └── ...
└── lv3/
    └── <product-name>/
        ├── README.md
        ├── simulation/
        └── ...
```

製品は原則として **現在のLevelのディレクトリに1つだけ存在** させます。  
昇格時にはコピーではなくディレクトリごと移動します。

## Product README

Lv1以上の各製品にはREADMEを置きます。

最低限、以下を記載します。

- 対応するIssue
- Current Level
- Summary
- Use case
- Interface / expected behavior
- 仕様
- 既知の制約
- 現在ある設計・実装
- 次のLevelへ上がるために不足しているもの

Issueは議論と履歴、製品READMEは現在確定している仕様と成果物の入口として使います。

## Simulation policy

Lv3の検証は、**オープンソースのシミュレーション環境で第三者が再現できること**を原則とします。

優先順位:

1. 用途に適した既存OSSシミュレータを利用する
2. 足りないデバイスモデル、BFM、アダプタ、テストベンチだけ追加する
3. 既存OSSで表現できない場合に限り、必要なシミュレータやモデルを新しく作る
4. プロプライエタリEDAは補助利用可だが、Lv3の必須条件にはしない

候補例:

| 対象 | OSS候補 |
| --- | --- |
| FPGA / RTL / デジタル回路 | Verilator + cocotb |
| 軽量なVerilog検証 | Icarus Verilog + cocotb |
| アナログ / 電源 / トランジスタ回路 | ngspice |
| MCU / SoC / Firmware込み | Renode |
| OS / PC / 仮想デバイス | QEMU |

USB、SATA、PCIe、特殊デバイスなど必要な相手側モデルが無ければ、まず既存シミュレータ上で再利用可能なモデルを追加します。

### Lv3 reproducibility

可能な限り、製品ディレクトリ内に次のような検証環境を残します。

```text
simulation/
├── README.md
├── run.sh / Makefile
├── tests/
├── models/
└── requirements.txt / pyproject.toml
```

少ない手順で実行でき、PASS / FAILが明確に分かることを目標にします。

GitHub Actions等で自動実行できる場合は、

> Lv3 = 過去に一度動いた

ではなく、

> **Lv3 = 現在のmainでも再現可能なシミュレーションが通っている**

状態を維持します。

## Workflow

1. 欲しいハードウェアをIssueにする — **Lv0**
2. Issueで用途・仕様を固める
3. 仕様が固まったら `Src/lv1/<product>/README.md` を作る — **Lv1**
4. 設計図または実装ソースができたら `Src/lv2/` へ移動 — **Lv2**
5. OSSシミュレーションが完了したら `Src/lv3/` へ移動 — **Lv3**

## First product

現在の最初の製品案:

- [#1 USB Mass Storage → SATA Device Bridge](https://github.com/tomiya7688/open-hard-ideas/issues/1)
- Current Level: **Lv2**
- Product directory: `Src/lv2/usb-storage-to-sata/`

---

**思いついたら、まず Issue へ。**
