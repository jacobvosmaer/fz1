# Casio FZ-1 utilities

- `akwf-fzv`: converts an AKWF 600-sample mono waveform into a Casio FZ-1 wave synth voice
- `fzformat`: creates an empty FZ-1 disk image
- `fzputfile`: stores a given file onto an FZ-1 disk image file
- `fzbuildfull`: assemble FZV voice files into an FZF full data dump

Read more in this blog post: https://blog.jacobvosmaer.nl/0057-fz-1-images/ .

## What is the Casio FZ-1

The Casio FZ-1 is a sampler/synthesizer keyboard from 1987. There also existed rack-mounted variants called FZ-10M and FZ-20M. The FZ-1 product line was also sold re-badged by Hohner as the HS-1.

## What are the tools in this repository for

The tools in this repository help me load files from my computer into the memory of the FZ-1.

The FZ-1 has a built-in 3.5" floppy drive. Nowadays it is common to replace the floppy drive with a floppy drive emulator. The emulator stores floppy disk images on a USB stick and the FZ-1 thinks it is reading from real floppies.

I use the tools in this repository to copy audio files from my computer onto a floppy disk image. I then put the USB stick in the floppy emulator and then the FZ-1 can load the files.
