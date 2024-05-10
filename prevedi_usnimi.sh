# Postavi putanje za arm-none-eabi-gcc i cmake
export PATH="/Applications/ARM/bin/:/usr/local/bin/:$PATH"

# Putanja do STM32_Programmer_CLI i projekta
STM32_CLI="/Users/miki/Applications/Applications/new/new/STM32CubeProgrammer.app/Contents/MacOs/bin/STM32_Programmer_CLI"
PRJ_PATH="/Volumes/WORK/401_blink/smart_knob"
cd $PRJ_PATH

if [ -d "build" ]; then
    rm -r build
fi

mkdir build && cd build

cmake -DCMAKE_C_FLAGS="-Wall -Wno-unused" ..
#cmake ..
make -j 8

# Flesuj firmware koristeći STM32_Programmer_CLI
$STM32_CLI -c port=SWD -q -w "$PRJ_PATH/build/smart_knob.elf" -rst
