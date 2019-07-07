###############################################################################################
## 
## Scripting LORABEE COMMANDS: <LORA> 1. Config 2. Loop RADIO TX + READ REPLIES (1 or 2)
#   http://www.swingnote.com/tools/texttohex.php

##
## => Use CYGWIN (not MSYS2!)
##

cd ~
MYFILE=lorabee_tx_init.sh
cat << 'THETEXTBLOCK' > ${MYFILE}

LORABEE=/dev/ttyS15 # COM17
ls -l ${LORABEE}

function func_lorabee_send_command()
{
    green='\033[32m'
    white='\033[37m'
    echo -e "\n${green}SENDING LORA COMMAND: ${1}${white}"
    # set 57600, 8 bits, 1 stop bit, no parity
    ~/picocom/picocom --baud 57600 --databits 8 --stopbits 1 --parity n  --flow n --omap crcrlf --echo ${LORABEE} \
        --initstring "$(echo -ne "${1}\r")" --quiet --exit-after 2000
}

func_lorabee_send_command ""  # clear buffer (possibly invalid data waiting). Ignore the output "invalid_param"

func_lorabee_send_command "sys reset"
func_lorabee_send_command "sys set pindig GPIO0 1"
func_lorabee_send_command "mac pause"
#####func_lorabee_send_command "radio set pwr -3"
func_lorabee_send_command "radio set pwr 10"
#####func_lorabee_send_command "radio set pwr 15"
func_lorabee_send_command "radio set mod lora"
func_lorabee_send_command "radio set freq 868910000"
func_lorabee_send_command "radio set sf sf7"
func_lorabee_send_command "radio set bw 125"
func_lorabee_send_command "radio set cr 4/8"
func_lorabee_send_command "radio set wdt 0"
#####func_lorabee_send_command "radio set wdt 1000"
func_lorabee_send_command "sys set pindig GPIO0 0"

THETEXTBLOCK
cat ${MYFILE}

cd ~
MYFILE=lorabee_tx_loop.sh
cat << 'THETEXTBLOCK' > ${MYFILE}

LORABEE=/dev/ttyS15
ls -l ${LORABEE}

function func_lorabee_send_command()
{
    green='\033[32m'
    white='\033[37m'
    echo -e "\n${green}SENDING LORA COMMAND: ${1}${white}"
    # set 57600, 8 bits, 1 stop bit, no parity
    ~/picocom/picocom --baud 57600 --databits 8 --stopbits 1 --parity n  --flow n --omap crcrlf --echo ${LORABEE} \
        --initstring "$(echo -ne "${1}\r")" --quiet --exit-after 2000
}

for MYSEQ in {0001..0100}
  do 
    func_lorabee_send_command "sys set pindig GPIO0 1"
    # xxxx0123456789ABCDEF 20 hex chars SEQ + HELLO WORLD BASH SCRIPT + test sequence
    func_lorabee_send_command "radio tx ${MYSEQ}48454C4C4F20574F524C442042415348205343524950540102030405060708090A0B0C0D0E0F"
    func_lorabee_send_command "radio tx ${MYSEQ}48454C4C4F20574F524C442042415348205343524950540102030405060708090A0B0C0D0E0F"
    func_lorabee_send_command "radio tx ${MYSEQ}48454C4C4F20574F524C442042415348205343524950540102030405060708090A0B0C0D0E0F"
    func_lorabee_send_command "sys set pindig GPIO0 0"
#    # read 2x non-blocking response (@important The TX command takes time to execute.)
#    MYRESPONSE=
#    read -r -t 1 MYRESPONSE < ${LORABEE}
#    printf "SEQ ${MYSEQ} | `date` %s\n" "${MYRESPONSE}"
#    MYRESPONSE=
#    read -r -t 2 MYRESPONSE < ${LORABEE}
#    printf "SEQ ${MYSEQ} | `date` %s\n" "${MYRESPONSE}"
done

THETEXTBLOCK
cat ${MYFILE}

chmod --verbose u+x lorabee*.sh

## Execute INIT -COPYPASTE#1-
./lorabee_tx_init.sh

## Execute LOOP -COPYPASTE#2-
./lorabee_tx_loop.sh
