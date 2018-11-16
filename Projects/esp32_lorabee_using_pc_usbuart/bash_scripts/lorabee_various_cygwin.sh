##
## <LORAWAN> Testing123
# CYGWIN>>
export LORABEE=/dev/ttyS15
ls -l ${LORABEE}
set +x
# set 57600, 8 bits, 1 stop bit, no parity
stty -F ${LORABEE} 57600 cs8 -echo -cstopb -parenb
stty -F ${LORABEE}
echo -e "mac save\r\n" > ${LORABEE}; read  -t 3 MYRESPONSE < ${LORABEE}; printf "`date` %s\n" "$MYRESPONSE"

##
## <LORA> GET INFO ***The response is not always returned due to timing issues (the Bash script is too slow to get the response)***
# CYGWIN>>
export LORABEE=/dev/ttyS15
set +x
ls -l ${LORABEE}
# set 57600, 8 bits, 1 stop bit, no parity
stty -F ${LORABEE} 57600 cs8 -echo -cstopb -parenb
stty -F ${LORABEE}
echo -e "sys reset\r\n" > ${LORABEE};       read  -t 1 MYRESPONSE < ${LORABEE}; printf "`date` %s\n" "$MYRESPONSE"
echo -e "sys get ver\r\n" > ${LORABEE};     read  -t 0.1 MYRESPONSE < ${LORABEE}; printf "`date` %s\n" "$MYRESPONSE"
echo -e "sys get hweui\r\n" > ${LORABEE};   read  -t 0.1 MYRESPONSE < ${LORABEE}; printf "`date` %s\n" "$MYRESPONSE"
echo -e "radio mac pause\r\n" > ${LORABEE};  read  -t 0.1 MYRESPONSE < ${LORABEE}; printf "`date` %s\n" "$MYRESPONSE"
echo -e "radio mac resume\r\n" > ${LORABEE}; read  -t 0.1 MYRESPONSE < ${LORABEE}; printf "`date` %s\n" "$MYRESPONSE"
echo -e "radio get mod\r\n" > ${LORABEE};    read  -t 0.1 MYRESPONSE < ${LORABEE}; printf "`date` %s\n" "$MYRESPONSE"
echo -e "radio get pwr\r\n" > ${LORABEE};    read  -t 0.1 MYRESPONSE < ${LORABEE}; printf "`date` %s\n" "$MYRESPONSE"
echo -e "radio get freq\r\n" > ${LORABEE};   read  -t 0.1 MYRESPONSE < ${LORABEE}; printf "`date` %s\n" "$MYRESPONSE"
echo -e "radio get sf\r\n" > ${LORABEE};     read  -t 0.1 MYRESPONSE < ${LORABEE}; printf "`date` %s\n" "$MYRESPONSE"
echo -e "radio get bw\r\n" > ${LORABEE};     read  -t 0.1 MYRESPONSE < ${LORABEE}; printf "`date` %s\n" "$MYRESPONSE"
echo -e "radio get wdt\r\n" > ${LORABEE};    read  -t 0.1 MYRESPONSE < ${LORABEE}; printf "`date` %s\n" "$MYRESPONSE"
echo -e "radio get snr\r\n" > ${LORABEE};    read  -t 0.1 MYRESPONSE < ${LORABEE}; printf "`date` %s\n" "$MYRESPONSE"

##
## @doc For your info some Microchip NVM commands
#   sys get nvm 300
#   sys get nvm 310
#   sys get nvm 320
#   sys get nvm 330
#   sys get nvm 340
#   sys get nvm 350
#   sys get nvm 360
#   sys get nvm 370
#   sys get nvm 380
#   sys get nvm 390
#   sys get nvm 3A0

