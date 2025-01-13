# CTRL-Modular-Macro-Keyboard

In today’s fast-paced digital workflows, professionals in fields such as graphic design, video editing, and 3D modeling often face challenges with traditional keyboards. Tasks like scrolling through timelines, adjusting settings, or switching tools require repetitive or complex key combinations, reducing efficiency and increasing frustration. Recognizing this need, we developed a modular macro keyboard—a customizable and intuitive solution designed to streamline workflows and enhance user experience.

The current system consists of 4 modules with potential for future expansion. The 3 functional modules are: the keymodule, the knobmodule and the slidermodule. Functional modules get power over the pogo-connectors and individually transmit data directly to the receiver hub using the ESP-Now protocol. This fourth receiver module is connected to the computer over USB and converts the wirelessly received data into keyboard and mouse input for the computer . 

The files provided are 4 individual arduino files for each of the modules, and the complementary AutoHotkey script that allows the profile switch to change between the programs displayed on the computer screen. In order for this function to work, the AutoHotkey program should be installed first. This can be done using the following link: https://www.autohotkey.com/

Once this is installed, the script can be activated by clicking the file. Adjustments to this file can be done using editors like VSC.

