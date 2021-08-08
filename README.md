# NSMBW Inspector
## About
NSMBW (New Super Mario Bros Wii.) Inspector is a tool that hooks into the game running on Dolphin and lists the currently loaded class instances. You can then view the data of an instance by clicking on it and manipulate fields defined in a structure file. This project is currently still very much a WIP and only very few classes are defined, but the hope is to add as many of them as possible.

NSMBW Inspector is built with electron and native modules compiled from [aldelaro5's Dolphin memory engine](https://github.com/aldelaro5/Dolphin-memory-engine). I might try moving to a different framework for the GUI at some point since electron generates ridiculously large binaries.
## Compatibility
The tool currently only supports Windows. Linux support is in consideration. For MacOS compatibility, the [Dolphin memory engine](https://github.com/aldelaro5/Dolphin-memory-engine) would have to receive MacOS support first.
## Installation
You can download the latest executable from the [releases](https://github.com/LetsPlentendo-CH/nsmbw-inspector/releases).
### Note for Linux users
If you're on Linux, you will have to run the following commands in the directory of the executable after you've downloaded it:
```shell
sudo setcap cap_sys_ptrace=eip nsmbwinspector
sudo chown root:root chrome-sandbox
sudo chmod 4755 chrome-sandbox
```
## Running from source
1. Clone the repository: `git clone --recursive https://github.com/LetsPlentendo-CH/nsmbw-inspector.git`
2. Navigate inside the project
3. Run `npm install`
4. To launch the app, run `npm start`

If you want to build an executable, run these additional steps (This might be required if you're on Linux):

5. deploy using `npm run make`
6. copy `structures.txt` from the root folder to `out/nsmbwinspector-win32-x64` (or `out/nsmbwinspector-linux-x64`, if you're on Linux)
7. (Linux only) Install `patchelf` and, in `out/nsmbwinspector-linux-x64`, run the following commands:
```shell
sudo setcap cap_sys_ptrace=eip nsmbwinspector
sudo chown root:root chrome-sandbox
sudo chmod 4755 chrome-sandbox
patchelf --set-rpath ./ nsmbwinspector # the first command removes some enviroment variables, we add RPATH back
```

## Structure files
NSMBW Inspector loads in a text file that describes classes and their fields on launch.

This is how the file is structured:
### General
The file type does not require indentation (or even line breaks, for that matter).
It contains of description blocks, which are each explained below.

These always have the following syntax:
```
<block name> <headers>
    <...>
end
```
### Structure
A structure block describes a data structure, such as that of a class. The header contains three parameters, seperated by colons:
`<structure name>:<structure/class from which this structure inherits>:<size of the structure>`
If the structure does not inherit from a different structure/class, this parameter should contain a single hyphen.
The structure size may be in decimal or in hexadecimal (in which case the number must be preceded by `0x`)

A structure block contains field definitions. These have the following syntax:
`<offset to start of structure> <field name>:<field data type>;`

The field data type may be one of the currently implemented basic types (see the [list](#basic-types) at the bottom) or the name of a different structure (This will then be a structure inside of another structure, not a pointer to it).

Example:
```
structure dBase:fBase:0xc
    0x0 firstLinkedNode:ptr;
    0x4 explanationString:stringJIS;
    0x8 nameString:string;
end
```
This example defines a structure named `dBase` which inherits from `fBase` and has size `0xc`. It contains three fields:

- `firstLinkedNode` with type `ptr` at offset `0x0`.
- `explanationString` with type `stringJIS` at offset `0x4`.
- `nameString` with type `string` at offset `0x8`.

### Display
A display block describes a list of fields of a structure which should be displayed when a structure is viewed. The header contains one parameter, the structure which this display refers to.

A display block contains displayers. These describe which field of the structure should be displayed in what style. A displayer has the following format:
`<displayer style>:"<title>":<field name>`

Note: The title must be enclosed in quotes.

Only the displayer style `textbox` is currently implemented. Future styles include, for example, `checkbox` which could be used for flag fields.

Example:
```
display dBase
    textbox:"Object Name":nameString;
    textbox:"Explanation String":explanationString;
end
```
This defines a display for the structure `dBase`. It contains two displayers:

- a `textbox` which contains the field `nameString` and has the title `Object Name`.
- a `textbox` which contains the field `explanationString` and has the title `Explanation String`.


### Preview
A preview block describes a list of fields of a structure which should be displayed when a structure is used as a data type in a different structure. The header contains one parameter, the structure which this preview refers to.

A preview block contains a previewer. The previewer describes which fields of the structure should be displayed. A previewer has the following format:
`<previewer style>:<field name 1>:<field name 2>:(...):<field name n>;`

Only the previewer style `multi` is currently implemented. This is just multiple textboxes beneath each other.

Example:
```
preview vec3
    multi:x:y:z;
end
```
This defines a preview for the structure `vec3`. The previewer has the type `multi` and shows the fields `x`, `y` and `z`.

### Basic types
Type | Description
--- | ---
s32 | 32-bit singed integer
u32 | 32-bit unsigned integer
ptr | 32-bit pointer
s16 | 16-bit signed integer
s16angle | 16-bit signed integer, converted to an angle in degrees (-32768 to 32767 -> -180° to 180°)
u16 | 16-bit unsigned integer
s8 | 8-bit signed integer
u8 | 8-bit unsigned integer
float | 32-bit floating-point value
string | pointer to an array of characters, terminated by \0
stringJIS | pointer to a string encoded in Shift JIS