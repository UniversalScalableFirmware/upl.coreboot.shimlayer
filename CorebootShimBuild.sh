#!/bin/bash
BuildTarget="DEBUG"

export WORKSPACE=$(cd `dirname $0`; pwd)
echo $WORKSPACE

while [ $# -gt 0 ]; do
  shift
done

cd $WORKSPACE/CorebootUplShimPkg
make cleanall
make
