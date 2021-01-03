const iconv = require("iconv-lite");

function u32ToSigned(value) {
    return value | 0; // idk how to do this
}

function u16ToSigned(value) {
    return (value << 16) >> 16;
}

function u8ToSigned(value) {
    return (value << 8) >> 8;
}

function signedToU32(value) {
    return value >>> 0; // idk how to do this
}

function signedToU16(value) {
    return (value << 16) >> 16 >>> 0;
}

function signedToU8(value) {
    return (value << 8) >> 8 >>> 0;
}

const getS32 = (offset) => (addr) => u32ToSigned(dr.readU32(addr + offset)).toString();
const getPointer = (offset) => (addr) => "0x" + dr.readU32(addr + offset).toString(16);
const getU32 = (offset) => (addr) => dr.readU32(addr + offset).toString();

const getS16 = (offset) => (addr) => u16ToSigned(dr.readU16(addr + offset)).toString();
const getS16Angle = (offset) => (addr) => (u16ToSigned(dr.readU16(addr + offset)) / 0xFFFF * 360).toFixed(2);
const getU16 = (offset) => (addr) => dr.readU16(addr + offset).toString();

const getS8 = (offset) => (addr) => u8ToSigned(dr.readU8(addr + offset)).toString();
const getU8 = (offset) => (addr) => dr.readU8(addr + offset).toString();

const getFloat = (offset) => (addr) => dr.readFloat(addr + offset).toFixed(2);

const getString = (offset) => (addr) => {
    let ptr = dr.readPointer(addr + offset);
    return arrayToString(dr.readValues(ptr, 64));
}

const getStringJIS = (offset) => (addr) => {
    let ptr = dr.readPointer(addr + offset);
    let str = arrayToString(dr.readValues(ptr, 64));
    for (let i = 0; i < str.length; i++) {
        if (str[i] == 0) {
            str = explStr.str(0, i);
        }
    }
    return iconv.decode(Buffer.from(str, "binary"), "Shift-JIS");
}

const writeS32 = (offset) => (addr, value) => dr.writeU32(addr + offset, signedToU32(Number(value)));
const writePointer = (offset) => (addr, value) => dr.writeU32(addr + offset, Number(value));
const writeU32 = (offset) => (addr, value) => dr.writeU32(addr + offset, Number(value));

const writeS16 = (offset) => (addr, value) => dr.writeU16(addr + offset, signedToU16(Number(value)));
const writeS16Angle = (offset) => (addr, value) => {
    let val = Math.floor(Number(value) / 360 * 0xFFFF);
    dr.writeU16(addr + offset, val);
};
const writeU16 = (offset) => (addr, value) => dr.writeU16(addr + offset, Number(value));

const writeS8 = (offset) => (addr, value) => dr.writeU8(addr + offset, signedToU8(Number(value)));
const writeU8 = (offset) => (addr, value) => dr.writeU8(addr + offset, Number(value));

const writeFloat = (offset) => (addr, value) => dr.writeFloat(addr + offset, parseFloat(value));

let displayFuncs = {
    "s32": getS32,
    "u32": getU32,
    "ptr": getPointer,
    "u16": getU16,
    "s16": getS16,
    "s16angle": getS16Angle,
    "u8": getU8,
    "float": getFloat,
    "string": getString,
    "stringJIS": getStringJIS
}

let writeFuncs = {
    "s32": writeS32,
    "u32": writeU32,
    "ptr": writePointer,
    "s16": writeS16,
    "s16angle": writeS16Angle,
    "u16": writeU16,
    "s8": writeS8,
    "u8": writeU8,
    "float": writeFloat
}

function removeAllowedSpaces(str) {
    str = str.replace(/\n|\r/g, "");
    let allowedToRemoveSpaces = true;
    for (let i = 0; i < str.length; i++) {
        if (str[i] == '"') {
            allowedToRemoveSpaces = !allowedToRemoveSpaces;
            str = str.slice(0, i) + str.slice(i + 1);
            i--;
        }
        if (str[i] == ' ' && allowedToRemoveSpaces) {
            str = str.slice(0, i) + str.slice(i + 1);
            i--;
        }
    }
    return str;
}

function parseStructures(data) {
    let structs = {};

    // first loop to get size and inheritation info from all structures
    for (let struct of data) {
        let structInfo = [...struct.matchAll(/^(.+):(.+):(.+)/g)][0];
        let structName = structInfo[1];
        let inheritsFrom = structInfo[2];
        if (inheritsFrom == "-") inheritsFrom = "";
        let structSize = Number(structInfo[3]);

        structs[structName] = {
            inheritsFrom: inheritsFrom,
            size: structSize,
            fields: {} // this is filled in the next step
        };
    }

    // second loop to get fields of structs
    for (let struct of data) {
        let fields = [...struct.matchAll(/([0-9a-fx]+?) ([:!\w\d\n\r]+?);/gm)]; // 0: entire match, 1: address, 2: name and type (seperated by ':')
        let structName = struct.match(/^(.+?)(?=:)/g)[0];
        
        // find offset to this structure from the given address
        let offset = 0;
        let currStruct = structs[structName];
        while(currStruct) {
            currStruct = structs[currStruct.inheritsFrom];
            if (currStruct) offset += currStruct.size;
        }

        for (let field of fields) {
            let addr = Number(field[1]);
            let name = field[2].split(":")[0];
            let type = field[2].split(":")[1];
            let isChangeable = false;
            if (type[0] == '!') {
                isChangeable = true;
                type = type.substring(1); // remove exclamation mark from type
            }

            let fieldObject = {
                type: type,
                offset: offset + addr,
                editable: isChangeable
            };
            structs[structName].fields[name] = fieldObject;
        }
    }
    return structs;
}

function parsePreviews(data) {
    let retPreviews = {};
    for (let preview of data) {
        let structName = preview.match(/^.+/g)[0];
        let matches = preview.match(/multi:[:!\w\d\n\r]+?;/gm);
        if (!matches) break;
        let previewInstrs = [...matches];
        let previews = previewInstrs.map(el => removeAllowedSpaces(el));
        previews = previews.map(el => el.slice(0, -1).split(":"));

        let previewFields = [];

        for (let p of previews) {
            let thisField = {};
            thisField.type = p[0];
            thisField.fields = [];

            switch(p[0]) {
                case "multi":
                    for (let i = 1; i < p.length; i++) { 
                        thisField.fields.push(p[i]);
                    }
                    break;
            }
            previewFields.push(thisField);
        }
        retPreviews[structName] = previewFields;
    }
    return retPreviews;
}

function parseDisplays(data) {
    let retDisplays = {};
    for (let disp of data) {
        let dispName = disp.match(/^.+/g)[0];
        let matches = disp.match(/(textbox|checkbox)(.|\n|\r)+?;/gm);
        if (!matches) break;
        let dispInstrs = [...matches];

        // format displays (remove spaces)
        let displays = dispInstrs.map(el => removeAllowedSpaces(el));
        displays = displays.map(el => el.slice(0, -1).split(":"));

        let inspectListData = [];

        for (let d of displays) {
            let thisField = {};
            thisField.type = d[0];
            thisField.title = d[1];

            thisField.fields = [];

            switch(d[0]) {
                case "textbox":
                    for (let i = 2; i < d.length; i++) {
                        thisField.fields.push(d[i]);
                    }
                    break;
                case "checkbox":
                    // TODO
                    break;
            }
            inspectListData.push(thisField);
        }
        retDisplays[dispName] = inspectListData;
    }
    return retDisplays;
}

function makeDisplayer(structName, displays, previews, structs) {
    let displayObjects = [];
    if (!structs[structName]) return [];
    let struct = structs[structName];
    if (!displays[structName]) return [];
    for (let display of displays[structName]) {
        let thisEl = {};
        thisEl.name = display.title;
        thisEl.type = "textbox";

        // now make the function
        let fieldName = display.fields[0]; // only one field per textbox currently
        if (!struct.fields[fieldName]) continue;
        let fieldOffset = struct.fields[fieldName].offset;
        let fieldType = struct.fields[fieldName].type;
        let getFunction;
        let writeFunction;
        if (displayFuncs[fieldType]) {
            getFunction = displayFuncs[fieldType](fieldOffset);
            if (struct.fields[fieldName].editable && writeFuncs[fieldType]) {
                writeFunction = writeFuncs[fieldType](fieldOffset);
            }
            thisEl.count = 1;
        } else if (previews[fieldType]) {
            let preview = previews[fieldType][0]; // only show first preview field for the moment
            if (preview.type != "multi") continue;
            thisEl.count = preview.fields.length;
            let dispFunctions = [];
            let writeFunctions = [];
            for (let f of preview.fields) {
                if (!structs[fieldType].fields[f]) continue;
                let innerFieldType = structs[fieldType].fields[f].type;
                let innerFieldOffset = structs[fieldType].fields[f].offset;
                if (displayFuncs[innerFieldType]) {
                    dispFunctions.push(displayFuncs[innerFieldType](fieldOffset + innerFieldOffset));
                }
                if (writeFuncs[innerFieldType]) {
                    writeFunctions.push(writeFuncs[innerFieldType](fieldOffset + innerFieldOffset));
                }
            }
            getFunction = (address) => {
                let res = [];
                for (let func of dispFunctions) {
                    res.push(func(address));
                }
                return res;
            };
            if (struct.fields[fieldName].editable) {
                writeFunction = (address, values) => {
                    let i = 0;
                    for (let func of writeFunctions) {
                        func(address, values[i]);
                        i++;
                    }
                };
            }
        } else {
            continue;
        }
        thisEl.get = getFunction;
        if (writeFunction) {
            thisEl.change = writeFunction;
        }
        displayObjects.push(thisEl);
    }
    return displayObjects;
}

function parseStructureFile(fileContents) {
    // a structure defines an actor by listing the offsets of field alongside with their names and types
    let structuresData = [...fileContents.matchAll(/structure ((.|\n|\r)+?)end/gm)];
    structuresData = structuresData.map(el => el[1]);
    
    let structs = parseStructures(structuresData);

    // a preview can be optionally defined for a structure.
    // it shows up when a not-inherited structures exists in another structure.
    let previewsData = [...fileContents.matchAll(/preview ((.|\n|\r)+?)end/gm)];
    previewsData = previewsData.map(el => el[1]);

    let previews = parsePreviews(previewsData);
    
    // a display definition lists the field of a structures which should be shown.
    let displayData = [...fileContents.matchAll(/display ((.|\n|\r)+?)end/gm)];
    displayData = displayData.map(el => el[1]);
    
    let displays = parseDisplays(displayData);

    // object definitions can be used to quickly define an actor without knowing the fields
    let objects = [...fileContents.matchAll(/^object (.+?);/gm)];
    objects = objects.map(el => el[1]);

    let inheritanceList = {};
    let inspectList = {};

    for (let struct in structs) {
        let displayer = makeDisplayer(struct, displays, previews, structs);
        if (displayer.length > 0) {
            inspectList[struct] = displayer;
        }
        inheritanceList[struct] = structs[struct].inheritsFrom;
    }

    for (let obj of objects) {
        let splitted = obj.split(":");
        inheritanceList[splitted[0]] = splitted[1];
    }

    return {
        inheritanceList: inheritanceList,
        inspectList: inspectList
    }
}

module.exports = {
    parseStructureFile
};