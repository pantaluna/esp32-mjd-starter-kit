###############################################################################################
## 
## Scripting LORABEE COMMANDS: <LORA> 1. Config 2. Loop RADIO RX + READ REPLIES (1 or 2)
#   http://www.swingnote.com/tools/texttohex.php

##
## => Use CYGWIN (not MSYS2!)
##

cd ~
MYFILE=lorabee_rx_init.sh
cat << 'THETEXTBLOCK' > ${MYFILE}

LORABEE=/dev/ttyS15 # COM16
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

chmod --verbose u+x lorabee*.sh

## Execute -COPYPASTE#1-
./lorabee_rx_init.sh

#######################################################
## RX LOOP (using picocom delay-exit)
LORABEE=/dev/ttyS15 # COM16
ls -l ${LORABEE}
function func_lorabee_radio_rx_command()
{
    # set 57600, 8 bits, 1 stop bit, no parity
    ~/picocom/picocom --baud 57600 --databits 8 --stopbits 1 --parity n  --flow n --omap crcrlf --echo ${LORABEE} \
        --initstring "$(echo -ne "radio rx 0\r")" --quiet --exit-after 1000
}

for i in {1..100}
do
    echo "."
    func_lorabee_radio_rx_command
done

#######################################################
# @tip When you are done then press ^A^X to exit picocom
LORABEE=/dev/ttyS15 # COM16
ls -l ${LORABEE}

~/picocom/picocom --baud 57600 --databits 8 --stopbits 1 --parity n  --flow n --omap crcrlf --echo ${LORABEE}

radio rx 0

