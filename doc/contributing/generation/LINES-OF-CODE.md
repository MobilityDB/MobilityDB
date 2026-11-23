<!--
  MobilityDB — Generation of the bindings: lines of code
  Copyright(c) MobilityDB Contributors

  This documentation is licensed under a Creative Commons Attribution-Share
  Alike 3.0 License: https://creativecommons.org/licenses/by-sa/3.0/
-->

# Lines of code of MobilityDB, MEOS and the bindings, 2026-11-23

[Back to the index](00-INDEX.md)

Counted by `tools/count_loc.sh` with cloc 1.90 on a fresh `git clone --depth 1`
of the default branch of each `github.com/MobilityDB/<repo>` (submodules not fetched, `.git`
excluded); each pull request of the generation plan runs it again. *Code* is cloc's code
column. *Code w/o data* leaves out the CSV and JSON rows, which are mostly test fixtures, sample
data or generated catalogs. MobilityNebula counts the lines its default branch adds or modifies
over its NebulaStream base, `nebulastream/nebulastream` at `6c63b6a` (2026-11-06).

## Summary

| Group | Repository | Commit | Files | Code | Code w/o data | Main language (code) |
|---|---|---|---:|---:|---:|---|
| Core | MobilityDB | `4fd2d9e` 2026-11-22 | 2467 | 1352696 | 958225 | C (439660), SQL (289258) |
| Programming languages | PyMEOS | `b6be02b` 2026-11-20 | 175 | 36449 | 36449 | Python (35666) |
|  | PyMEOS-CFFI | `1cab8bd` 2026-11-21 | 23 | 43929 | 43929 | Python (43257) |
|  | JMEOS | `da6409b` 2026-11-20 | 185 | 31480 | 31480 | Java (26460) |
|  | MEOS.NET | `66cfb9d` 2026-11-20 | 289 | 56597 | 56597 | C# (50323) |
|  | MEOS.js | `31be640` 2026-11-20 | 111 | 80530 | 74953 | TypeScript (55900), C (17090) |
|  | GoMEOS | `726dcc9` 2026-11-20 | 265 | 488033 | 88483 | Go (86870) |
|  | meos-rs | `2b49f27` 2026-10-31 | 65 | 29915 | 29915 | Rust (29586) |
| Batch and stream processors | MobilityDuck | `396579b` 2026-11-21 | 293 | 240719 | 129668 | C++ (116675) |
|  | MobilitySpark | `82783da` 2026-11-19 | 18 | 2596 | 2596 | Java (2004) |
|  | MobilityFlink | `6c9f71c` 2026-11-20 | 220 | 172781 | 15906 | Java (13261) |
|  | MobilityKafka | `9b08ffb` 2026-11-11 | 60 | 4169 | 3968 | Java (3178) |
|  | MobilityNebula (without NebulaStream) | `c836014` 2026-11-23 | 166 | 265232 | 264250 | C++ (254345) |
| MobilityAPI | MobilityAPI | `65cbbff` 2026-11-21 | 105 | 22625 | 13635 | Go (11510) |
|  | MobilityAPI-Python | `6439451` 2026-11-12 | 74 | 88749 | 86247 | Python (84795) |
| MEOS-API | MEOS-API | `8f8950b` 2026-11-21 | 137 | 21508 | 18496 | Python (15843) |
| Benchmark | MobilityDB-BerlinMOD | `2b841e1` 2026-11-08 | 67 | 23754 | 23754 | SVG (12865), SQL (5307) |
| Index extension | mest | `26503db` 2026-11-21 | 262 | 105139 | 105139 | C (73051), SQL (22387) |
| Workshop | MobilityDB-workshop | `329b2ab` 2026-06-25 | 16 | 3823 | 3823 | XML (3243) |

| Group | Code | Code w/o data |
|---|---:|---:|
| Core | 1352696 | 958225 |
| Programming languages | 766933 | 361806 |
| Batch and stream processors | 685497 | 416388 |
| MobilityAPI | 111374 | 99882 |
| MEOS-API | 21508 | 18496 |
| **All bindings** | **1585312** | **896572** |
| Benchmark | 23754 | 23754 |
| Index extension | 105139 | 105139 |
| Workshop | 3823 | 3823 |
| **Total** | **3070724** | **1987513** |

## Notes

- **Data files:** MobilityDB has 391846 lines of CSV, GoMEOS 399490, MobilityFlink 156875
  and MobilityDuck 110322. These are test and sample data, which is why the *w/o data* column exists.
- **MobilityNebula** is counted on `main`, its only branch since 2026-11-22 (the former
  `next`), without the NebulaStream engine it carries: the count is the lines `main` adds or
  modifies over its upstream base, `nebulastream/nebulastream` at `6c63b6acb1` (2026-11-06,
  on NebulaStream `main`), from `cloc --diff <nebulastream> <MobilityNebula>`. By `git diff
  --numstat`, 264728 added lines are the MEOS plugin (`nes-plugins/MEOS`, generated from the
  MEOS catalog), 28198 its generators (`tools`), 4458 system tests (`nes-systests`), and
  4678 the NebulaStream core (operators, SQL parser, query compiler, data types). The full
  tree is 478911 lines of code, 212879 of them unchanged NebulaStream. The count was 240723
  on the former `next` at `6cb506b` (2026-11-21), and 272147 on `main` at `75a4b41` before
  #280 removed the earlier release's operators and query files, which no build listed.
- **MobilityAPI-Python:** cloc counts 456076 notebook *comment* lines, which are
  stored cell outputs. The code count is not affected.
- **Generated code:** a binding's count holds its generated files with its written ones; how much of
  each binding is generated is measured in the [index](00-INDEX.md), table *How much of each binding
  is generated*.
- **MobilityDB-workshop** is mostly prose: its 3243 XML lines are the seven DocBook chapters
  of the workshop (AIS, AIS dashboard, flight data dashboard, GPX, GTFS, location history,
  and the book file), with their SQL listings inline. cloc counts 16 of its 71 tracked
  files; the others are 52 PNG figures, 2 zipped datasets and a LaTeX style file.

## Per-repository cloc output

### MobilityDB

```
--------------------------------------------------------------------------------
Language                      files          blank        comment           code
--------------------------------------------------------------------------------
C                               888          56247         219097         439660
CSV                              20              1              0         391846
SQL                             718          26686          44154         289258
XML                              62           3084            408          92602
C/C++ Header                    402           9491          37668          82939
YAML                             62            589           1460          15941
Python                           53           2587          10327          12253
Markdown                         53           2437              0          10003
SVG                              18             52             14           5249
CMake                           105            617           3816           3946
JSON                             35              0              0           2625
C++                               4            214            302           1698
Perl                             14            259            379           1694
Bourne Shell                     15            126            880           1043
yacc                              1             67             33            460
Pascal                            1              0           1329            450
CSS                               1             80             45            319
m4                                2             76             79            271
make                              5             61             32            149
lex                               1             20             32            101
Bourne Again Shell                1             12             62             75
XSLT                              3             14             34             58
Dockerfile                        1             11             11             40
TeX                               2              7             19             16
--------------------------------------------------------------------------------
SUM:                           2467         102738         320181        1352696
--------------------------------------------------------------------------------
```

### PyMEOS

```
-------------------------------------------------------------------------------
Language                     files          blank        comment           code
-------------------------------------------------------------------------------
Python                         146           9528          16156          35666
YAML                             4             57             26            319
reStructuredText                17            187            263            206
Markdown                         4             46              0            136
TOML                             2             13              6             87
DOS Batch                        1              8              1             26
make                             1              4              7              9
-------------------------------------------------------------------------------
SUM:                           175           9843          16459          36449
-------------------------------------------------------------------------------
```

### PyMEOS-CFFI

```
-------------------------------------------------------------------------------
Language                     files          blank        comment           code
-------------------------------------------------------------------------------
Python                          13          10837            364          43257
YAML                             4             89             55            529
TOML                             2              8              6             64
Markdown                         3             22              0             62
Bourne Shell                     1              3             15             17
-------------------------------------------------------------------------------
SUM:                            23          10959            440          43929
-------------------------------------------------------------------------------
```

### JMEOS

```
-------------------------------------------------------------------------------
Language                     files          blank        comment           code
-------------------------------------------------------------------------------
Java                           168           7058          17581          26460
Python                           2            445           1596           2481
Markdown                         5            420              0           1837
Maven                            2             38             21            367
YAML                             4             48             71            207
XML                              2              2              0             71
Dockerfile                       1             11             24             34
Bourne Shell                     1              4             51             23
-------------------------------------------------------------------------------
SUM:                           185           8026          19344          31480
-------------------------------------------------------------------------------
```

### MEOS.NET

```
------------------------------------------------------------------------------------
Language                          files          blank        comment           code
------------------------------------------------------------------------------------
C#                                  276          18032           1028          50323
Markdown                              3             97              0           4691
Python                                3            378           2189           1419
MSBuild script                        4             16              5             59
YAML                                  1             13             18             47
Visual Studio Solution                1              1              1             41
Bourne Shell                          1              3             15             17
------------------------------------------------------------------------------------
SUM:                                289          18540           3256          56597
------------------------------------------------------------------------------------
```

### MEOS.js

```
-------------------------------------------------------------------------------
Language                     files          blank        comment           code
-------------------------------------------------------------------------------
TypeScript                      85          11474           8895          55900
C                                1           4254             60          17090
JSON                             6              0              0           5577
Markdown                        14            629              0           1568
Dockerfile                       1             39            127            234
YAML                             2             27             41             98
JavaScript                       1              7             21             46
Bourne Shell                     1              3             15             17
-------------------------------------------------------------------------------
SUM:                           111          16433           9159          80530
-------------------------------------------------------------------------------
```

### GoMEOS

```
-------------------------------------------------------------------------------
Language                     files          blank        comment           code
-------------------------------------------------------------------------------
CSV                              9              0              0         399490
Go                             241          17284          14185          86870
Python                           3            304           1353           1175
Markdown                         3             55              0            252
YAML                             2             22             50             93
JSON                             1              0              0             60
C/C++ Header                     1              4              1             28
C                                1              7              1             25
XML                              3              0              0             23
Bourne Shell                     1              3             15             17
-------------------------------------------------------------------------------
SUM:                           265          17679          15605         488033
-------------------------------------------------------------------------------
```

### meos-rs

```
-------------------------------------------------------------------------------
Language                     files          blank        comment           code
-------------------------------------------------------------------------------
Rust                            55           1212           3937          29586
Markdown                         4             77              0            170
TOML                             4             13              0             89
YAML                             1             20              0             67
C/C++ Header                     1              0              0              3
-------------------------------------------------------------------------------
SUM:                            65           1322           3937          29915
-------------------------------------------------------------------------------
```

### MobilityDuck

```
-------------------------------------------------------------------------------
Language                     files          blank        comment           code
-------------------------------------------------------------------------------
C++                             78           6604           4589         116675
CSV                             24              0              0         110322
Python                          11            622           6495           5961
Markdown                        18            651              0           2419
C/C++ Header                    66            671           2558           2174
SQL                             56            215           2186            914
JSON                            23              1              0            729
Jupyter Notebook                 2              0           7321            658
CMake                            6             54            332            328
YAML                             3             29            182            209
SVG                              1              1              1            156
Bourne Shell                     4             37            166            137
make                             1              7             34             37
-------------------------------------------------------------------------------
SUM:                           293           8892          23864         240719
-------------------------------------------------------------------------------
```

### MobilitySpark

```
-------------------------------------------------------------------------------
Language                     files          blank        comment           code
-------------------------------------------------------------------------------
Java                            11            153            502           2004
Markdown                         2             72              0            192
SVG                              1              1              1            156
Maven                            1              6             18            138
YAML                             1             15             45             65
XML                              1              0              8             24
Bourne Shell                     1              4             36             17
-------------------------------------------------------------------------------
SUM:                            18            251            610           2596
-------------------------------------------------------------------------------
```

### MobilityFlink

```
-------------------------------------------------------------------------------
Language                     files          blank        comment           code
-------------------------------------------------------------------------------
CSV                              2              0              0         156875
Java                           182           2608           5729          13261
Markdown                         8            334              0           1180
Maven                            3             27             44            485
Bourne Shell                    12             52             52            377
YAML                             4             39             73            313
SVG                              1              1              1            156
Dockerfile                       4             18             25             59
Python                           2             17             33             45
XML                              1              0              8             24
SQL                              1              1              0              6
-------------------------------------------------------------------------------
SUM:                           220           3097           5965         172781
-------------------------------------------------------------------------------
```

### MobilityKafka

```
-------------------------------------------------------------------------------
Language                     files          blank        comment           code
-------------------------------------------------------------------------------
Java                            49            524           2014           3178
Markdown                         4             77              0            285
Maven                            3             18             13            276
CSV                              1              0              0            201
SVG                              1              1              1            156
YAML                             1             14             31             56
Bourne Shell                     1              3             14             17
-------------------------------------------------------------------------------
SUM:                            60            637           2073           4169
-------------------------------------------------------------------------------
```

### MobilityNebula (`cloc --diff` over NebulaStream `6c63b6a` (2026-11-06); counted: added + modified)

```
--------------------------------------------------------------------------------
Language                      files          blank        comment           code
--------------------------------------------------------------------------------
C++
 same                           536          12672          10118          80114
 modified                        21              0              0            100
 added                           65          10929           1206         254245
 removed                          0              0              0             34
CSV
 same                            45              1              0          56720
 modified                         0              0              0              0
 added                            0              0              0              0
 removed                          0              0              0              0
C/C++ Header
 same                           560           9202          11015          33511
 modified                        15              0              1             11
 added                           24            376            595           1687
 removed                          0              0              0              1
Markdown
 same                            57           1931              0           8317
 modified                         3              0              0              1
 added                            3             49              0            286
 removed                          3             43              0            122
Python
 same                             8            238            215            922
 modified                         0              0              0              0
 added                            7           1959           8582           7387
 removed                          1             50             45            132
CMake
 same                           230           1903           3443           8221
 modified                        10              0              0              0
 added                            5             30             68            136
 removed                          0              0              0              0
YAML
 same                            64            316            695           3223
 modified                         2              0              2             50
 added                            2             29             47            231
 removed                         37            258            173           2997
JSON
 same                            17              0              0           3957
 modified                         0              0              0              0
 added                            3              0              0            982
 removed                          0              0              0              0
diff
 same                            38            286           1325           4861
 modified                         0              0              0              0
 added                            0              0              0              0
 removed                          0              0              0              0
Rust
 same                            16            352            979           2959
 modified                         0              0              0              0
 added                            0              0              0              0
 removed                          0              0              0              0
Nix
 same                            19            181             13           2886
 modified                         1              0              0              7
 added                            0              1              9             13
 removed                          0              0              0              0
Bourne Shell
 same                            19            353            500           2033
 modified                         2              0              1              0
 added                            1              2             39             66
 removed                          0              0              1              0
ANTLR Grammar
 same                             0              0             26            517
 modified                         1              0              0              5
 added                            0              3              4             13
 removed                          0              0              0              0
Dockerfile
 same                             7             48             84            388
 modified                         0              0              0              0
 added                            1              1              5             12
 removed                          0              0              0              0
Bourne Again Shell
 same                             1             56             82            324
 modified                         0              0              0              0
 added                            0              0              0              0
 removed                          0              0              0              0
Sass
 same                             1             28              0            162
 modified                         0              0              0              0
 added                            0              0              0              0
 removed                          0              0              0              0
Protocol Buffers
 same                             6             42             70            133
 modified                         0              0              0              0
 added                            0              0              0              0
 removed                          0              0              0              0
SQL
 same                            12             10              5            120
 modified                         0              0              0              0
 added                            0              0              0              0
 removed                          0              0              0              0
TOML
 same                             7             35             70            120
 modified                         0              0              0              0
 added                            0              0              0              0
 removed                          0              0              0              0
Pascal
 same                             1             13             11             96
 modified                         0              0              0              0
 added                            0              0              0              0
 removed                          0              0              0              0
HTML
 same                            10              0              0             69
 modified                         0              0              0              0
 added                            0              0              0              0
 removed                          0              0              0              0
--------------------------------------------------------------------------------
SUM:
 same                          1654          27667          28651         209653
 modified                        55              0              4            174
 added                          111          13379          10555         265058
 removed                         41            351            219           3286
--------------------------------------------------------------------------------
```

### MobilityAPI

```
-------------------------------------------------------------------------------
Language                     files          blank        comment           code
-------------------------------------------------------------------------------
Go                              59            829           2257          11510
JSON                            19              7              0           8990
Markdown                        12            182              0            659
TypeScript                       3             43             72            469
Jupyter Notebook                 2              0           1052            272
YAML                             2             32            110            257
SQL                              2             25             83            155
Java                             2             18             20            138
HTML                             2              0              0            100
Python                           1              6             11             51
Bourne Shell                     1              2              7             24
-------------------------------------------------------------------------------
SUM:                           105           1144           3612          22625
-------------------------------------------------------------------------------
```

### MobilityAPI-Python

```
-------------------------------------------------------------------------------
Language                     files          blank        comment           code
-------------------------------------------------------------------------------
Python                          57           1195          29862          84795
JSON                             3              0              0           2502
Jupyter Notebook                 3              0         456076            690
Markdown                         7            121              0            216
Bourne Shell                     1             18             12            175
SVG                              1              1              1            156
SQL                              1             20             51            133
YAML                             1              7             18             82
-------------------------------------------------------------------------------
SUM:                            74           1362         486020          88749
-------------------------------------------------------------------------------
```

### MEOS-API

```
-------------------------------------------------------------------------------
Language                     files          blank        comment           code
-------------------------------------------------------------------------------
Python                         102           3636           8792          15843
JSON                            10             10              0           3012
Markdown                        11            342              0           1709
YAML                             6             71            167            585
Bourne Shell                     8             47            242            359
-------------------------------------------------------------------------------
SUM:                           137           4106           9201          21508
-------------------------------------------------------------------------------
```

### MobilityDB-BerlinMOD

```
-------------------------------------------------------------------------------
Language                     files          blank        comment           code
-------------------------------------------------------------------------------
SVG                              7              1            156          12865
SQL                             24            993           2558           5307
XML                              5            196              0           2917
Markdown                        13            325              0           1075
Bourne Shell                    10             99            212            713
Python                           3             90             83            415
CSS                              1             79             34            316
YAML                             2             20             11            111
Dockerfile                       1              5              5             29
TeX                              1              2              6              6
-------------------------------------------------------------------------------
SUM:                            67           1810           3065          23754
-------------------------------------------------------------------------------
```

### mest

```
-------------------------------------------------------------------------------
Language                     files          blank        comment           code
-------------------------------------------------------------------------------
C                              156          14411          18137          73051
SQL                             39           3353           3566          22387
C/C++ Header                    46           1275           4233           8337
yacc                             1             67             29            447
CMake                            6             70            100            407
Markdown                         7             83              0            220
YAML                             3             15             12            105
lex                              1             24             22            105
Pascal                           1             11              0             58
make                             2             10              2             22
-------------------------------------------------------------------------------
SUM:                           262          19319          26101         105139
-------------------------------------------------------------------------------
```

### MobilityDB-workshop

```
-------------------------------------------------------------------------------
Language                     files          blank        comment           code
-------------------------------------------------------------------------------
XML                              7            115              0           3243
CSS                              1             79             34            316
YAML                             2             18             10            118
Markdown                         3             48              0            109
Dockerfile                       1              5              0             18
Bourne Shell                     1              7              0             13
TeX                              1              2              6              6
-------------------------------------------------------------------------------
SUM:                            16            274             50           3823
-------------------------------------------------------------------------------
```
