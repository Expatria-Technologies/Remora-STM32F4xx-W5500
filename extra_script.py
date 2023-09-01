Import("env")

def after_build(source, target, env): 
    print("[From Script] Finished building!!")
    env.Replace(PROGNAME="FLEXI_HAL_%s" % env.GetProjectOption("custom_prog_version")+"_"+env.GetProjectOption("grblhal_driver_version"))
    env.Execute("python uf2conv.py -c -b 0x08010000 -f 0x57755a57 $BUILD_DIR/firmware.hex --output ${PROGNAME}.uf2")

env.AddPostAction("buildprog", after_build)

