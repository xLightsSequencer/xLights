To set up Piano effect:

1. op new seq
2. add a row
3. select a display element; example piano keyboard looks okay with a 32x32 grid
4. set Piano effect
a. set Style = Piano Keys
b. set #keys; example shapes look okay with 3 pixels wide
c. set #rows (scroll or eq bars only)
d. placement is not used yet
5. set Cues to onsets file; there are a couple of little Piano-Test files in bin folder
6. Set Cue shape map file; there are examples in the bin folder (use 88Keys for piano keyboard)
7. set Shapes file; there are examples in the bin folder
7b. clip will chop off any keys that won't fit within the display; off will wrap (in which case you probably want a multiple of 12 keys to fit within the display element)
8. set color palette; colors in the example shapes files are mapped sequentially to palette colors (then wraps); for the example piano shapes file, if you set 4 palette colors to dark red, dark green, bright red, bright green, this will preserve the colors in the example shapes file
9. update grid, save, play

Notes:
- Key tops seems to work fairly well (square or rectangular grid); key edges needs a little adjustment yet to get the key up + down effect to look better (should be a short + wide rectangular grid)
- scrolling piano looks a little odd due to the way the example key shapes overlap (use square or rectangular grid).  probably the best way to solve this is to use alternate shapes that do not overlap
- eq bars is crude; it should look at amplitude, but just picks a random number for now to simulate the effect