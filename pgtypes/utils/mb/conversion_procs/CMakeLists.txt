set(CONVERSION_PROCS_SOURCES
  ${CMAKE_CURRENT_SOURCE_DIR}/big5.c
  ${CMAKE_CURRENT_SOURCE_DIR}/cyrillic_and_mic.c
  ${CMAKE_CURRENT_SOURCE_DIR}/euc2004_sjis2004.c
  ${CMAKE_CURRENT_SOURCE_DIR}/euc_cn_and_mic.c
  ${CMAKE_CURRENT_SOURCE_DIR}/euc_jp_and_sjis.c
  ${CMAKE_CURRENT_SOURCE_DIR}/euc_kr_and_mic.c
  ${CMAKE_CURRENT_SOURCE_DIR}/euc_tw_and_big5.c
  ${CMAKE_CURRENT_SOURCE_DIR}/latin2_and_win1250.c
  ${CMAKE_CURRENT_SOURCE_DIR}/latin_and_mic.c
  ${CMAKE_CURRENT_SOURCE_DIR}/proc.mk
  ${CMAKE_CURRENT_SOURCE_DIR}/sjis.map
  ${CMAKE_CURRENT_SOURCE_DIR}/utf8_and_big5.c
  ${CMAKE_CURRENT_SOURCE_DIR}/utf8_and_cyrillic.c
  ${CMAKE_CURRENT_SOURCE_DIR}/utf8_and_euc2004.c
  ${CMAKE_CURRENT_SOURCE_DIR}/utf8_and_euc_cn.c
  ${CMAKE_CURRENT_SOURCE_DIR}/utf8_and_euc_jp.c
  ${CMAKE_CURRENT_SOURCE_DIR}/utf8_and_euc_kr.c
  ${CMAKE_CURRENT_SOURCE_DIR}/utf8_and_euc_tw.c
  ${CMAKE_CURRENT_SOURCE_DIR}/utf8_and_gb18030.c
  ${CMAKE_CURRENT_SOURCE_DIR}/utf8_and_gbk.c
  ${CMAKE_CURRENT_SOURCE_DIR}/utf8_and_iso8859.c
  ${CMAKE_CURRENT_SOURCE_DIR}/utf8_and_iso8859_1.c
  ${CMAKE_CURRENT_SOURCE_DIR}/utf8_and_johab.c
  ${CMAKE_CURRENT_SOURCE_DIR}/utf8_and_sjis.c
  ${CMAKE_CURRENT_SOURCE_DIR}/utf8_and_sjis2004.c
  ${CMAKE_CURRENT_SOURCE_DIR}/utf8_and_uhc.c
  ${CMAKE_CURRENT_SOURCE_DIR}/utf8_and_win.c
)

# Aggregate with child contributions
set(ALL_SOURCES
  ${PGTYPES_SOURCES}  # contains mb/ and mb/conversion files
  ${CONVERSION_PROCS_SOURCES}
)

# Push once to parent
set(PGTYPES_SOURCES ${ALL_SOURCES} PARENT_SCOPE)
