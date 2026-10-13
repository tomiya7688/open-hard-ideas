# open-hard-ideas

> 「こんなハードウェアがあったらいいな」を Issue に投げ、仕様・設計・シミュレーションまで育てる場所。

このリポジトリは、ハードウェア製品のアイデアを集め、成熟度に応じて **Lv0 → Lv1 → Lv2 → Lv3** と昇格させていくためのリポジトリです。

基本単位は **1 Issue = 1製品案** です。

## Levels

| Level | 状態 | 成果物 |
| --- | --- | --- |
| **Lv0 — Idea / Spec Draft** | アイデアから仕様を固めている途中 | Issue上の議論・要件・技術調査 |
| **Lv1 — Specified** | 主要仕様が確定し、設計へ入れる | 製品README / 仕様書 |
| **Lv2 — Design / Implementation** | 設計または実装が一通り成立している | 設計図、回路図、KiCadデータ、組み込みソース、HDL等 |
| **Lv3 — Open Simulation Verified** | シミュレーション段階が完了 | 再現可能なOSSシミュレーションとPASS結果 |

Levelは「何かファイルが存在するか」ではなく、**現在どこまで完成しているか** を表します。

技術検証用の断片コード、スパイク、未完成の回路図などは、次のLevelへ昇格する根拠にはしません。

### Lv0 — Idea / Spec Draft

思いついた案はまずIssueにします。

用途、必要なI/O、期待動作、制約、既存製品との差などを議論し、製品仕様を固めます。

この段階では技術調査や試作コードを書いて構いませんが、それらは仕様・設計の完成を意味しません。

**Lv0では原則として `Src/` に製品を設置しません。**

### Lv1 — Specified

製品の主要仕様が固まり、実装者がREADMEを読んで、重要な仕様を勝手に決め足さず設計へ入れる状態です。

少なくとも、その製品で重要な項目について次が決まっていることを目安にします。

- 目的 / Use case
- 入出力インターフェース
- 対応規格・世代
- 電源条件
- 必須機能
- 対応範囲
- 非対応範囲
- エラー時の基本動作
- 実機で満たすべき受け入れ条件

この時点で、

```text
Src/lv1/<product-name>/README.md
```

を作成します。

READMEはその製品の仕様書兼入口です。

### Lv2 — Design / Implementation

Lv1の仕様を満たすための設計または実装が、**設計成果物として一通り成立している**段階です。

例:

- 回路図が一通り完成している
- KiCad等のEDAデータが設計として成立している
- 主要部品と接続が決定している
- FPGA / HDLの主要実装が完成している
- MCU firmware / 組み込みコードの主要機能が実装されている
- 機構設計が一通り完成している

部分的なreference implementationや技術スパイクだけではLv2にしません。

Lv2へ昇格したら、製品ディレクトリごと移動します。

```text
Src/lv1/<product-name>/
        ↓
Src/lv2/<product-name>/
```

### Lv3 — Open Simulation Verified

Lv2の設計について、主要な期待動作のシミュレーションが完了した段階です。

OSSのシミュレーション環境で第三者が検証を再現でき、PASS / FAILを確認できる状態を目標とします。

Lv3へ昇格したら、

```text
Src/lv2/<product-name>/
        ↓
Src/lv3/<product-name>/
```

へ移動します。

**Lv3は実機での動作保証を意味しません。**

## Repository layout

`Src` という名前ですが、software sourceだけを意味しません。

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

Lv0の案はIssue上に置き、仕様確定後に初めて `Src/lv1/` へ入れます。

## Product README

Lv1以上の各製品にはREADMEを置きます。

最低限、以下を記載します。

- 対応するIssue
- Current Level
- Summary
- Use case
- Interface / expected behavior
- 確定した仕様
- 対応範囲 / 非対応範囲
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
4. 設計または実装が一通り成立したら `Src/lv2/` へ移動 — **Lv2**
5. OSSシミュレーションが完了したら `Src/lv3/` へ移動 — **Lv3**

## Current ideas

- [#1 USB Mass Storage → SATA Device Bridge](https://github.com/tomiya7688/open-hard-ideas/issues/1) — **Lv0**

---

**思いついたら、まず Issue へ。**
