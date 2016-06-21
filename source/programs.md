Just copy these programs into the source code!

TODO: make something to pick from these inside the main program..

Quick JS script for getting rid of GARBAGE EW

```js
`(code)`
.replace(/0x|,|\w+/g,'')
```

Quick JS script for taking opcodes and making them into stuff you can pass to main.cpp:

```js
var vals = `(code)`
.replace(/(\w{1,2})/g, '0x$1,')
.split(' ')
console.log(`unsigned int size = ${vals.length*2+4};\nunsigned short buffer[size] = {${vals.join('\n')}};`)
```

------

# Clock test.

Waits for clock to finish after 240 ticks. Change value 0x60 is set to on
line 200 to change how many ticks it'll wait. Keep in mind that the program
will probably be running at 60 ticks per second.

```c++
  unsigned int size = 12;
  unsigned short buffer[size] = {
    /* 200 */ 0x60,0xF0,  // set V0 = 240
    /* 202 */ 0xF0,0x15,  // set timer = V0
    /* 204 */ 0xF0,0x07,  // set V0 = timer
    /* 206 */ 0x30,0x00,  // skip if V0 is 0
    /* 208 */ 0x12,0x04,  // go to 204
    /* 20A */ 0x6E,0x01,  // set VE = 1
  };
```

# Sprite test.

First test project for rendering a sprite.

```c++
  unsigned int size = 0x8;
  unsigned short buffer[size] = {
    /* 200 */ 0xA2,0x06,
    /* 202 */ 0xD0,0x02, // draw sprite @ (0,0) using height = 2
    /* 204 */ 0x12,0x08, // jump past sprite graphics
    /* 206 */ 0x3c,0x7e, // sprite graphics
  };
```

# Sprite test #2

Just another sprite test.

```c++
  unsigned int size = 0x13;
  unsigned short buffer[size] = {
    /* 200 */ 0x60,0x00, // V0 = 0
    /* 202 */ 0x61,0x01, // V1 = 1
    /* 204 */ 0xA2,0x0E, // I = 20C
    /* 206 */ 0x00,0xE0, // clear screen
    /* 208 */ 0x80,0x14, // V0 += V1
    /* 20A */ 0xD0,0x04, // draw @ V0 V0
    /* 20C */ 0x12,0x06, // jump to 204
    /* 20E */ 0xE7,0x81, // sprite graphics
    /* 211 */ 0x81,0xFF, // sprite graphics
  };
```
