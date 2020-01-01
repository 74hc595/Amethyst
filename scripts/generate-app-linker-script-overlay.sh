#!/bin/sh

SECTION=$1
shift
START="ADDR(${SECTION})+SIZEOF(${SECTION})"
DATA_AT="AT(LOADADDR(.data)+SIZEOF(.data))"
for app in $@
do
  echo ".app_${app}_data ${START} : ${DATA_AT} { "
  echo "  build/apps/${app}/*(.data)"
  echo "  build/apps/${app}/*(.data*)"
  echo "  build/apps/${app}/*(.gnu.linkonce.d*)"
  echo "  build/apps/${app}/*(.rodata)"
  echo "  build/apps/${app}/*(.rodata*)"
  echo "  build/apps/${app}/*(.gnu.linkonce.r*)"
  echo "} > data"
  echo "__data_load_start_app_${app} = LOADADDR(.app_${app}_data);"
  echo "__data_load_start_xpage_app_${app} = LOADADDR(.app_${app}_data) >> 16;"
  echo "__data_start_app_${app} = ADDR(.app_${app}_data);"
  echo "__data_end_app_${app} = ADDR(.app_${app}_data)+SIZEOF(.app_${app}_data);"
  echo ".app_${app}_bss ADDR(.app_${app}_data)+SIZEOF(.app_${app}_data) (NOLOAD) : { KEEP(build/apps/${app}/*(.bss*)) } > data"
  echo "__bss_end_app_${app} = ADDR(.app_${app}_bss)+SIZEOF(.app_${app}_bss);"
  echo ".app_${app}_noinit ADDR(.app_${app}_bss)+SIZEOF(.app_${app}_bss) (NOLOAD) : { KEEP(build/apps/${app}/*(.noinit*)) } > data"
  DATA_AT="AT(LOADADDR(.app_${app}_data)+SIZEOF(.app_${app}_data))"
done

