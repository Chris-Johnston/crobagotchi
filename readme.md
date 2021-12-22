# Crobagotchi

Raise your own _virtual_ crow!

<!-- TODO add image here -->

## What?

![Image](sprites/judgement%20dithered.bmp)

_Crobagotchi_ combines [Tamagotchi] with the [Crow of Judgement] (also referred to as a "Crob").

It's a portable handheld virtual pet which runs on an ATTiny85 and uses a SSD1306 OLED display
powered by a coin cell battery. The custom boards were designed using KiCad and [svg2mod], and the sprite work was done with Aseprite, GIMP, and [image2cpp].

## Why?

[Last year I made a PCB Crow of Judgement](https://github.com/Chris-Johnston/CrowOfJudgement), and this year I wanted to iterate on that. Eventually, I got the idea to make a knockoff Tamagotchi, and so I figured this would be a great time to do so.

![Image of the Crow of Judgement from 2020.](https://github.com/Chris-Johnston/CrowOfJudgement/raw/master/img/greetings.png)

[I still did make a simpler ornament this year, in the shape of an snowflake.](https://github.com/Chris-Johnston/snowflake-ornament) A lof of the board design is similar, it also uses ATTiny85.

## What else?

Battery life was a key concern, and so for this project I put a fair bit of time in to make sure
that wouldn't be a problem.

- Data is saved to EEPROM, so if the battery dies the state is preserved.
- Features of the ATTiny85 are disabled when unused, like the ADC.
- I use the deep sleep mode combined with the Watchdog Timer Interrupt to track realtime seconds (ticks once every 8 seconds), so that real time gets tracked as the device is sleeping.
- The SSD1306 is shut off as well during sleep to conserve power.

The other big issue was flash storage. The ATTiny85 has 8k of flash, which is largest offered.
I did not implement any compression for sprite bitmaps, which for each byte represents 8 vertical pixels. The 48x48 pixel image of the crow is 288 bytes, there were several times when testing where I used literally every last byte available in flash.

This code is pretty messy, I have not put much effort into cleaning it up as that was not a concern.

### SSD1306 Trouble

My experience with the SSD1306 with ATTiny85 was interesting. I tried a few libraries over time, and eventually settled with [Tiny4kOLED]. This one offered a sensible library that introduce extra fluff, like a menu that I'd never use?

Most importantly, I found that there was a bit-bang i2c implementation available where I could
reassign the pins. This was very important, because I had already ordered boards which relied
on a different pinout (PCBWay took a solid month to order), and I didn't want to disconnect
the display each time I reflashed it.

I copied this bitbang implementation and modified it to use the pins I specified.

Turns out there was more than one modification I'd make to this library. It included a 6x8 px font
which was great, but there were characters that I didn't use. Because of this, I modified
this font to add extra symbols that I'd use elsewhere, without having to add more to the flash.

[Tamagotchi]: https://en.wikipedia.org/wiki/Tamagotchi
[Crow of Judgement]: https://knowyourmeme.com/memes/crow-of-judgement
[svg2mod]: https://github.com/svg2mod/svg2mod
[image2cpp]: https://javl.github.io/image2cpp/
[Tiny4kOLED]: https://github.com/datacute/Tiny4kOLED