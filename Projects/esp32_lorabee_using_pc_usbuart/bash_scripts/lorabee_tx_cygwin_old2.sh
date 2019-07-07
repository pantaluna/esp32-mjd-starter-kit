
##
## <LORA> LOOP: RADIO TX + READ REPLY  ***ERROR: REPLY RARELY CATCHED BECAUSE THE RESPONSE COMES TO FAST FOR Bash***
#   http://www.swingnote.com/tools/texttohex.php
# CYGWIN>>
cd
MYFILE=lorabee_tx.sh
cat << 'THETEXTBLOCK' > ${MYFILE}
export LORABEE=/dev/ttyS15
set -x
ls -l ${LORABEE}
# set 57600, 8 bits, 1 stop bit, no parity
stty --file ${LORABEE} 57600 cs8 -echo -cstopb -parenb
stty --file ${LORABEE}
echo -e "sys reset\r\n" > ${LORABEE}; sleep 1;
echo -e "sys set pindig GPIO0 0\r\n" > ${LORABEE}; sleep 1;
echo -e "sys set pindig GPIO0 1\r\n" > ${LORABEE}; sleep 1;
echo -e "mac pause\r\n" > ${LORABEE}; sleep 1;
echo -e "radio set mod lora\r\n" > ${LORABEE}; sleep 1;
#####echo -e "radio set pwr 14\r\n" > ${LORABEE}; sleep 1;
echo -e "radio set pwr -3\r\n" > ${LORABEE}; sleep 1;
echo -e "radio set freq 865100000\r\n" > ${LORABEE}; sleep 1;
echo -e "radio set sf sf7\r\n" > ${LORABEE}; sleep 1;
echo -e "radio set bw 125\r\n" > ${LORABEE}; sleep 1;
echo -e "radio set wdt 0\r\n" > ${LORABEE}; sleep 1;
#####echo -e "radio set wdt 1000\r\n" > ${LORABEE}; sleep 1;
set +x
for MYSEQ in {0001..9999}
  do 
    # xxxx0123456789ABCDEF 20 hex chars SEQ + HELLO WORLD BASH SCRIPT + test sequence
    echo -e "radio tx ${MYSEQ}48454C4C4F20574F524C442042415348205343524950540102030405060708090A0B0C0D0E0F\r\n" > ${LORABEE}
    # read 2x non-blocking response (@important The TX command takes time to execute.)
    MYRESPONSE=
    read -r -t 1 MYRESPONSE < ${LORABEE}
    printf "SEQ ${MYSEQ} | `date` %s\n" "${MYRESPONSE}"
    MYRESPONSE=
    read -r -t 2 MYRESPONSE < ${LORABEE}
    printf "SEQ ${MYSEQ} | `date` %s\n" "${MYRESPONSE}"
done
THETEXTBLOCK
cat ${MYFILE}
chmod u+x lorabee*.sh

## Execute
./lorabee_tx.sh
