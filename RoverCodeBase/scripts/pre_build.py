Import("env")
import os

def before_build(source, target, env):
    # Create Arduino-style library structure if it doesn't exist
    lib_dir = os.path.join(env.get('PROJECT_DIR'), 'lib')
    if not os.path.exists(lib_dir):
        os.makedirs(lib_dir)

    # Create symlinks to Arduino libraries if needed
    arduino_lib_dir = os.path.expanduser('~/Documents/Arduino/libraries')
    if os.path.exists(arduino_lib_dir):
        for lib in os.listdir(arduino_lib_dir):
            src = os.path.join(arduino_lib_dir, lib)
            dst = os.path.join(lib_dir, lib)
            if not os.path.exists(dst) and os.path.isdir(src):
                try:
                    os.symlink(src, dst)
                except:
                    print(f"Could not create symlink for {lib}")

env.AddPreAction("buildprog", before_build) 