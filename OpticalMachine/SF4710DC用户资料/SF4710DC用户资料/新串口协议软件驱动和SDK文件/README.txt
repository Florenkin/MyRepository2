===============================================================================
                    固件烧录说明
===============================================================================

重要提示：请仔细阅读以下说明，确保正确进行固件烧录和设备操作。


1. 烧录方法
-------------------

   A. 直接硬件编程 (JLink/STLink)
   ----------------------------------------------
   首次编程设备或从固件损坏中恢复时使用此方法。
   
   - 要编程的文件：
     sf4710_iap_app_20250619.hex
     
   - 操作步骤：
     1. 将编程设备(JLink或STLink)连接到目标板
     2. 打开编程软件(如J-Flash、STM32CubeProgrammer)
     3. 选择HEX文件：sf4710_iap_app_20250619.hex
     4. 执行编程过程


   B. 通过GUI工具进行应用内编程(IAP)
   --------------------------------------------------
   用于正常固件更新和维护。
   
   - 要编程的文件：
     sf4710_app_20250619.bin
     
   - 操作步骤：
     1. 确保设备已通电并连接到您的电脑
     2. 启动SF GUI tool.exe
     3. 导航到固件更新部分
     4. 选择BIN文件：sf4710_app_20250619.bin
     5. 按照屏幕上的说明完成更新


2. 兼容性注意事项
-----------------------
   - HEX文件同时包含引导加载程序和应用固件
   - BIN文件仅包含应用固件
   - 这两个文件在两种编程方法之间不可互换
   - 使用错误的文件可能导致编程失败或设备故障


3. 故障排除
------------------
   如果在编程过程中遇到问题：
   1. 验证所有连接是否牢固
   2. 确保为您的编程方法选择了正确的文件
   3. 检查您的编程工具是否安装了最新驱动程序
   4. 如果问题仍然存在，请联系技术支持

===============================================================================
                    FIRMWARE FLASHING INSTRUCTIONS
===============================================================================

IMPORTANT: Please follow these instructions carefully to ensure proper 
firmware flashing and device operation.


1. FLASHING METHODS
-------------------

   A. Direct Hardware Programming (JLink/STLink)
   ----------------------------------------------
   Use this method when programming the device for the first time or 
   recovering from a firmware corruption.
   
   - File to Program:
     sf4710_iap_app_20250619.hex
     
   - Instructions:
     1. Connect the programming device (JLink or STLink) to the target board
     2. Open your programming software (e.g., J-Flash, STM32CubeProgrammer)
     3. Select the HEX file: sf4710_iap_app_20250619.hex
     4. Execute the programming process


   B. In-Application Programming (IAP) via GUI Tool
   --------------------------------------------------
   Use this method for normal firmware updates and maintenance.
   
   - File to Program:
     sf4710_app_20250619.bin
     
   - Instructions:
     1. Ensure the device is powered on and connected to your PC
     2. Launch SF GUI tool.exe
     3. Navigate to the firmware update section
     4. Select the BIN file: sf4710_app_20250619.bin
     5. Follow the on-screen instructions to complete the update


2. COMPATIBILITY NOTICE
-----------------------
   - The HEX file contains both the bootloader and application firmware
   - The BIN file contains only the application firmware
   - These files are NOT INTERCHANGEABLE between the two programming methods
   - Using the wrong file may result in programming failure or device malfunction


3. TROUBLESHOOTING
------------------
   If you encounter issues during programming:
   1. Verify all connections are secure
   2. Ensure the correct file is selected for your programming method
   3. Check that your programming tool has the latest drivers installed
   4. If problems persist, contact technical support

===============================================================================