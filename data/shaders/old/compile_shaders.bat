@REM cd shaders/
T:\android\code\libs\VulkanSDK\Bin\glslc.exe default.vert -o compiled/default.vert.spv -g -O0 -DHAS_NORMAL
T:\android\code\libs\VulkanSDK\Bin\glslc.exe default.frag -o compiled/default.frag.spv -g -O0 -DHAS_NORMAL
T:\android\code\libs\VulkanSDK\Bin\glslc.exe normaldebug.geom -o compiled/normaldebug.geom.spv -g -O0
T:\android\code\libs\VulkanSDK\Bin\glslc.exe normaldebug.vert -o compiled/normaldebug.vert.spv -g -O0
@REM cd ..
rem pause
