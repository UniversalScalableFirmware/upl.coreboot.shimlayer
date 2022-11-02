# upl.coreboot.shimlayer
Helper Code for coreboot to load payload that comply Universal Payload Interface

## Build Requirements
- make (GNU Make 4.2.1)
- clang (clang version 10.0.0-4ubuntu1)
- nasm (NASM version 2.15.05)
- llvm-ar (LLVM version 10.0.0)

## How to build Shimlayer
Create a new directory <workspace>, and using the below command  
```  
cd <workspace>  
./CorebootShimBuild.sh  
```  

ShimLayer.elf is generated in ```<workspace>/Build``` folder.  

## How to replace ShimLayer and UniversalPayload
Please refer to https://github.com/coreboot/coreboot to build coreboot.  
After building coreboot, you can use coreboot tool ```cbfstool``` to replace ShimLayer and UniversalPayload to ```coreboot.rom``` .  
Copy ```ShimLayer.elf``` and the target ```UniversalPayload.elf``` to ```<coreboot_workspace>/build```.  
Use the below command  
```
cd <coreboot_workspace>/build
./cbfstool coreboot.rom remove -r COREBOOT -n fallback/payload
./cbfstool coreboot.rom add-payload -r COREBOOT -n fallback/payload -f ShimLayer.elf
./cbfstool coreboot.rom remove -r COREBOOT -n img/UniversalPayload
./cbfstool coreboot.rom add-flat-binary -r COREBOOT -n img/UniversalPayload -f UniversalPayload.elf -l 0x200000 -e 0x100 -c lzma
```
Then the ```coreboot.rom``` has been replaced to your ```ShimLayer.elf``` and the target ```UniversalPayload.elf``` now.  
