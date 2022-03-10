import subprocess
import in_place

def get_commit_hash():
    result = subprocess.run(
        ["git", "rev-parse", "--short", "HEAD"],
        stdout=subprocess.PIPE,
        stderr=subprocess.DEVNULL,
        check=False
    )
    return result.stdout.decode("utf-8").rstrip() 

version = "nightly-" + get_commit_hash();
mod = "static const wxString xlights_qualifier       = \" " + version + "\";\n";
other = "#define Other \"_" + version + "\"\n";

with in_place.InPlace("../../xLights/xLightsVersion.h") as file:
    for line in file:
        if "xlights_qualifier" in line:
            line =  mod
        file.write(line)

with in_place.InPlace("xLights_common.iss") as file:
    for line in file:
        if "#define Other" in line:
            line = other
        file.write(line)