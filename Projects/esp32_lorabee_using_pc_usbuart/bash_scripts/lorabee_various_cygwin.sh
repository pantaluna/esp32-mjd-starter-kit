###
### Scripting the Lora commands
#     @doc Use these scripts when you are using Cygwin Bash. It works great!
#     @doc Use the QCOM Terminal programme from Quectel for MS Windows if you want to use a GUI App. It works great!

##
## picocom (OK)
#    @doc A good scriptable serial console for UART devices (GPS, LORA) and microcontrollers.
#    @doc ^A^X to exit
#    @doc --exit
#    @doc --exit-after 1000  
#    @doc --omap crcrlf so that pressing the ENTER key (or sending the '\r' key) sends a CR + LF to the device (essential for the LoraBee's Microchip RN2483A, else it does not process the command)

##
## => Use CYGWIN (not MSYS2!)
##

# [ONCE]
cd ~
git clone https://github.com/npat-efault/picocom; cd picocom && make && strip picocom;
cd ~/picocom
ls -l
man ./picocom.1

# Start terminal session (enter commands manually and exit using ^A^X)
LORABEE=/dev/ttyS15
ls -l ${LORABEE}
cd ~/picocom
./picocom --baud 57600 --databits 8 --stopbits 1 --parity n  --flow n --omap crcrlf --echo ${LORABEE}

## 
## Scripting LORABEE COMMANDS: picocom automation
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

func_lorabee_send_command ""  # clear buffer (possibly invalid data waiting). Ignore the output "invalid_param"
#reset
func_lorabee_send_command "sys reset"
#gpio
func_lorabee_send_command "sys set pindig GPIO0 1"
func_lorabee_send_command "sys set pindig GPIO0 0"
func_lorabee_send_command "sys set pindig GPIO0 1"
#info
func_lorabee_send_command "sys get vdd"
func_lorabee_send_command "sys get ver"
func_lorabee_send_command "sys get hweui"
#NVM
func_lorabee_send_command "sys get nvm 300"
func_lorabee_send_command "sys get nvm 310"
func_lorabee_send_command "sys get nvm 320"
func_lorabee_send_command "sys get nvm 330"
func_lorabee_send_command "sys get nvm 340"
func_lorabee_send_command "sys get nvm 350"
func_lorabee_send_command "sys get nvm 360"
func_lorabee_send_command "sys get nvm 370"
func_lorabee_send_command "sys get nvm 380"
func_lorabee_send_command "sys get nvm 390"
func_lorabee_send_command "sys get nvm 300"
func_lorabee_send_command "sys get nvm 3A0"
#radio
func_lorabee_send_command "radio get mod"
func_lorabee_send_command "radio get pwr"
func_lorabee_send_command "radio get freq"
func_lorabee_send_command "radio get sf"
func_lorabee_send_command "radio get bw"
func_lorabee_send_command "radio get wdt"
func_lorabee_send_command "radio get snr"
#mac ***
func_lorabee_send_command "mac resume"
func_lorabee_send_command "mac pause"
func_lorabee_send_command "mac save"


##
## radio TX: see the Bash script lorabee_tx_cygwin.sh


##
## radio RX: see the Bash script lorabee_rx_cygwin.sh


##
## [DO NOT USE] `stty` and `echo` and `read`
##   @problem The RX response is not always returned due to timing issues on the side of the Bash script (it is too slow to get the response using `read` after `echo`).
LORABEE=/dev/ttyS15
ls -l ${LORABEE}
set +x
# set 57600, 8 bits, 1 stop bit, no parity, do not echo input characters
stty --file ${LORABEE} 57600 cs8 -cstopb -parenb -echo 
stty --file ${LORABEE}
echo -e "sys get hweui\r\n" > ${LORABEE}; read  -t 5 MYRESPONSE < ${LORABEE}; printf "`date` %s\n" "$MYRESPONSE"


##
## [DO NOT USE] `screen` does not work!
##   @problem Cannot automate serial communication with scripting.
##
