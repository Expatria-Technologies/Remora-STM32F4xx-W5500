Import("env")

# Custom HEX from ELF
env.AddPostAction(
    "$BUILD_DIR/${PROGNAME}.elf",
    env.VerboseAction(" ".join([
        "$OBJCOPY", "-O", "ihex", "-R", ".eeprom", 
        '"$BUILD_DIR/${PROGNAME}.elf"', '"$BUILD_DIR/${PROGNAME}.hex"'
    ]), "Building $BUILD_DIR/${PROGNAME}.hex")
)

def after_build(source, target, env): 
    print("[From Script] Finished building!!")
    env.Replace(PROGNAME="FLEXI_HAL_%s" % env.GetProjectOption("custom_prog_version")+"_"+env.GetProjectOption("remora_driver_version"))
    env.Execute("python uf2conv.py -c -b 0x08010000 -f 0x57755a57 $BUILD_DIR/firmware.hex --output ${PROGNAME}.uf2")

env.AddPostAction("buildprog", after_build)

