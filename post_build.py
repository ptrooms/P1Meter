import os
Import("env")  # type: ignore : Import is not defined in pylance 

# using: https://forum.pjrc.com/index.php?threads/useful-script-for-generating-objdump-results-from-platformio.73834/

#https://docs.platformio.org/en/latest/projectconf/advanced_scripting.html
#https://docs.platformio.org/en/latest/projectconf/section_env_build.html#projectconf-dynamic-build-flags
#https://docs.platformio.org/en/latest/scripting/actions.html

# extensa: https://github.com/jcmvbkbc/gcc-xtensa

def after_build(source, target, env):
    platform = env.PioPlatform()

    """ 
    Run objdump on the target elf file and save the output in the top dir.
    """
    # see for variables: 
    #   /home/pafoxp/code-P1Meter/.pio/testcheck_platform-espressif8266-1.7.0/builder/main.py
    #   /home/pafoxp/code-P1Meter/.pio/testcheck_platform-espressif8266-1.6.0/builder/main.py
    print("ptro objcopy: " + env.subst("${OBJCOPY}"))
    print("ptro SIZETOOL: " + env.subst("${SIZETOOL}"))
    print("ptro FRAMEWORK_ARDUINOESP8266_DIR: " + env.subst("${FRAMEWORK_ARDUINOESP8266_DIR}")  )
    
    
    objdump=env.subst("${OBJCOPY}").replace("esptool","objdumpx")
    print("ptro objdump old: " + objdump)

    objdump=env.subst("${FRAMEWORK_ARDUINOESP8266_DIR}") + "/../toolchain-xtensa/bin/xtensa-lx106-elf-objdump"
    objdump="xtensa-lx106-elf-objdump"
    print("ptro objdump new: " + objdump)
    
    src_elf=env.subst("\"${BUILD_DIR}/${PROGNAME}.elf\"")
    src_lst=env.subst("\"${BUILD_DIR}/${PROGNAME}.asm\"")

    #--disassembler-options=no-aliases,numeric
    # objdump = esptool
    print("ptro src_elf: " + src_elf)
    #         "/home/pafoxp/.platformio/packages/toolchain-xtensa/xtensa-lx106-elf/bin/objdump", "-d", "--file-start-context", "-w",
    cmd=" ".join([
        objdump, "-d -C", 
        src_elf,">",src_lst]) #
    #--disassembler-options=no-aliases,numeric
    print("cmd=" + cmd)
    env.Execute(cmd)


env.AddCustomTarget(
    "disasm",
    "${BUILD_DIR}/${PROGNAME}.elf",
    after_build,
    title="Disasm 2",
    description="Generate a disassembly file on demand",
    always_build=True
)


