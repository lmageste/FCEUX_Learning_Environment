# Prefix:
PREFIX=/home/mageste/Desktop/tcc/DeepQNetwork/torch

$PREFIX/bin/luarocks make
RET=$?; if [ $RET -ne 0 ]; then echo "Error. Exiting."; exit $RET; fi
echo "FCEUX installation completed"
