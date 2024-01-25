![Logo](/readme_images/logo_sm.jpg)
# Remora-STM32F4xx-W5500
Port of Remora to uFlexiNet+FlexiHAL

# Installation instructions
This firmware uses a basically unchanged version of the Remora-nv ethernet that is located here:

https://github.com/Expatria-Technologies/Remora-NVEM

All testing is done with the Flexi-Pi LinuxCNC image from here:

https://github.com/Expatria-Technologies/remora-flexi-hal

From your linuxcnc home folder, start by cloning the Remora-NVEM repo and move to the Remora-nv component folder.
```
git clone https://github.com/Expatria-Technologies/Remora-NVEM.git
cd Remora-NVEM/LinuxCNC/Components/Remora-nv
```

Compile the component using halcompile
```
sudo halcompile --install remora-nv.c
```

Then upload your configuration file to the FlexiHAL using the upload_config.py script that should be in your configuration folder:
```
python3 upload_config.py FlexiHAL-config.txt
```
A basic configuration to get started is located here: 

https://github.com/Expatria-Technologies/linuxcnc_configs/tree/main/flexi-hal-uFlexiNet

All credit to Scotta and Terje IO as this project draws heavily from Remora (especially the RP2040 and STM32-W5500 ports) and also GRBLHAL (especially for the SPI DMA and networking stack)

<img src="/readme_images/Board_installed.jpg" width="800">
