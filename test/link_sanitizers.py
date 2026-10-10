Import("env")

# Sanitizer flags must also reach the linker for the native test build.
env.Append(LINKFLAGS=["-fsanitize=address,undefined"])
