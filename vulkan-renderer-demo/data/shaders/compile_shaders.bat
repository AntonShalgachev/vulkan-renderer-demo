cd shaders/
C:\VulkanSDK\1.2.154.1\Bin32\glslc.exe default.vert -o bin/default.vert.spv -g -O0 -DHAS_NORMAL
C:\VulkanSDK\1.2.154.1\Bin32\glslc.exe default.frag -o bin/default.frag.spv -g -O0 -DHAS_NORMAL
C:\VulkanSDK\1.2.154.1\Bin32\glslc.exe normaldebug.geom -o bin/normaldebug.geom.spv -g -O0
C:\VulkanSDK\1.2.154.1\Bin32\glslc.exe normaldebug.vert -o bin/normaldebug.vert.spv -g -O0
cd ..
rem pause
