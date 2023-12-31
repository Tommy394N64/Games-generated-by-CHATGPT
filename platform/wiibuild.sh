#!/bin/bash

get_abs_path()
{
    echo "$(cd "$1" && pwd)"
}

targets=( eduke32 mapster32 )
extensions=( ".dol" )

sourcedir="$(dirname "${BASH_SOURCE[0]}")"
sourcedir="$(get_abs_path "$sourcedir/..")"

pushd "${sourcedir}" >/dev/null

# Detect versioning systems and pull the revision number:
export VC_REV=$(svn info 2> /dev/null | grep Revision | awk '{ print $2 }')
vc=svn
if [ -z "$VC_REV" ]; then
    vc=git
    export VC_REV=$(git svn info 2> /dev/null | grep Revision | awk '{ print $2 }')
fi
if [ -z "$VC_REV" ]; then
    export VC_REV=Unknown
    vc=none
fi

date=$(date +%Y%m%d)

make=( make PLATFORM=WII $* STRIP="" SDLCONFIG="" )

echo "${make[@]}"
"${make[@]}"

for i in "${targets[@]}"; do
    for j in "${extensions[@]}"; do
        if [ ! -f "$i$j" ]; then
            exit 1
        fi
    done
done

# Package data:
mkdir -p apps

for i in "${targets[@]}"; do
    cp -R "platform/Wii/apps/$i" "apps/"
    for j in "${extensions[@]}"; do
        mv -f "$i$j" "apps/${i}/boot${j}"
    done
    for j in ".elf.map"; do
        rm -f "$i$j"
    done
    echo -e "    <version>r${VC_REV}</version>\n    <release_date>${date}</release_date>" | cat "platform/Wii/${i}_meta_1.xml" - "platform/Wii/${i}_meta_2.xml" >"apps/${i}/meta.xml"
done

if [ -d "apps/eduke32" ]; then
    cp -R package/common/* apps/eduke32/
    rm -f apps/eduke32/*.dll
fi

if [ -d "apps/mapster32" ]; then
    cp -R package/common/* apps/mapster32/
    cp -R package/sdk/* apps/mapster32/
    rm -f apps/mapster32/*.dll
fi

ls -l -R apps

rm -f "eduke32-wii-r${VC_REV}-debug-elf.7z"
rm -f "eduke32-wii-r${VC_REV}.7z"
7zr a -mx9 -ms=on -t7z "eduke32-wii-r${VC_REV}-debug-elf.7z" *.elf -xr!*.svn*
7zr a -mx9 -ms=on -t7z "eduke32-wii-r${VC_REV}.7z" apps -xr!*.svn*

popd >/dev/null
