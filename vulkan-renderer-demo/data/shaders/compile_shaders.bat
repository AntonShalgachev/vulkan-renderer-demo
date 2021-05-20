cd shaders/
C:\VulkanSDK\1.2.154.1\Bin32\glslc.exe default.vert -o compiled/default.vert.spv -g -O0 -DHAS_NORMAL
C:\VulkanSDK\1.2.154.1\Bin32\glslc.exe default.frag -o compiled/default.frag.spv -g -O0 -DHAS_NORMAL
C:\VulkanSDK\1.2.154.1\Bin32\glslc.exe normaldebug.geom -o compiled/normaldebug.geom.spv -g -O0
C:\VulkanSDK\1.2.154.1\Bin32\glslc.exe normaldebug.vert -o compiled/normaldebug.vert.spv -g -O0
cd ..
rem pause
