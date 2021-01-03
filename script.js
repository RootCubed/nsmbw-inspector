require("nan");
const dr = require("./build/Release/dr.node");

const fs = require("fs");

const disp = require("./instanceView.js");

let hook = false;

const PTMF_CREATE_PTR =  0x80377d20;
const PTMF_EXECUTE_PTR = 0x80377d34;
const PTMF_DRAW_PTR =    0x80377d48;
const PTMF_DELETE_PTR =  0x80377d5c;

const PTMFS = [
    PTMF_CREATE_PTR,
    PTMF_EXECUTE_PTR,
    PTMF_DRAW_PTR,
    PTMF_DELETE_PTR
];

let currPTMFList = PTMF_DRAW_PTR;

let prevPTMFHash = 0;
let currentlySelectedPTMF = [];

let currentInspectFunc;

const mainLeft = document.getElementById("mainLeft");
const mainRight = document.getElementById("mainRight");

structuresText = fs.readFileSync("structures.txt").toString();

let {inheritanceList, inspectList} = disp.parseStructureFile(structuresText);

document.getElementById("header-hook").addEventListener("click", () => {
    if (hook) return;
    if (dr.hook() == 0) {
        hook = true;
        startWorking();
    } else {
        alert("Failed to hook to Dolphin. Make sure you have started up a game.");
    }
});

document.querySelectorAll(".header-label").forEach(el => {
    el.addEventListener("click", () => {
        for (let tab of document.getElementsByClassName("header-label")) {
            if (tab.classList.contains("selected")) tab.classList.remove("selected");
        }
        el.classList.add("selected");
        
        let index = [...el.parentNode.children].indexOf(el);
        currPTMFList = PTMFS[index];
    });
});

mainLeft.addEventListener("keydown", ev => {
    if (ev.key == "ArrowDown") {
        ev.preventDefault();
        let els = document.getElementById("mainLeft").children;
        let csP = currentlySelectedPTMF[1];
        if (csP < els.length - 1) {
            els[csP].classList.remove("selected");
            els[csP + 1].classList.add("selected");
            if (els[csP + 1].getBoundingClientRect().bottom + 20 > window.innerHeight) {
                els[csP + 1].scrollIntoView(false);
            }
            currentlySelectedPTMF[0] = parseInt(els[csP + 1].children[0].innerText, 16);
            currentlySelectedPTMF[1]++;
            selectInstance(currentlySelectedPTMF[0]);
        }
    }
    if (ev.key == "ArrowUp") {
        ev.preventDefault();
        let els = document.getElementById("mainLeft").children;
        let csP = currentlySelectedPTMF[1];
        if (csP > 0) {
            els[csP].classList.remove("selected");
            els[csP - 1].classList.add("selected");
            if (els[csP - 1].getBoundingClientRect().top - 20 < 0) {
                els[csP - 1].scrollIntoView(true);
            }
            currentlySelectedPTMF[0] = parseInt(els[csP - 1].children[0].innerText, 16);
            currentlySelectedPTMF[1]--;
            selectInstance(currentlySelectedPTMF[0]);
        }
    }
});

mainLeft.addEventListener("click", ev => {
    if (ev.target == mainLeft) return;
    if (ev.target.classList.contains("PTMFRemove")) {
        removePTMF(parseInt(ev.target.parentNode.dataset.ptmf, 16));
        return;
    }
    if (currentlySelectedPTMF[1] >= 0 && mainLeft.children[currentlySelectedPTMF[1]]) {
        mainLeft.children[currentlySelectedPTMF[1]].classList.remove("selected");
    }

    let clickedPTMF = ev.target;
    if (clickedPTMF.nodeName != "DIV") {
        clickedPTMF = clickedPTMF.parentNode;
    }
    clickedPTMF.classList.add("selected");

    let index = [...clickedPTMF.parentNode.children].indexOf(clickedPTMF);

    currentlySelectedPTMF = [parseInt(clickedPTMF.children[0].innerText, 16), index];

    selectInstance(currentlySelectedPTMF[0]);
});

function readMemory() {
    let pointerToUse = currPTMFList;

    let beginNode = dr.readPointer(pointerToUse);
    let endNode = dr.readPointer(pointerToUse + 4);

    let functionListHTML = "";

    if (beginNode == 0 || endNode == 0) {
        prevPTMFHash = 0;
        mainLeft.innerHTML = "";
        return;
    }

    let count = 0;

    let newPMTFHash = "";

    let foundSelected = false;

    while (beginNode != 0) {
        let realfBase = dr.readPointer(beginNode + 8);
        let namePtr = dr.readPointer(realfBase + 0x6C);
        let instanceName = arrayToString(dr.readValues(namePtr, 32));

        if (!foundSelected && realfBase == currentlySelectedPTMF[0]) {
            currentlySelectedPTMF[1] = count;
            newPMTFHash += hashString(count.toString());
            functionListHTML += makePTMFHTMLSelected(beginNode, realfBase, instanceName);
            updateInstance(realfBase);
            foundSelected = true;
        } else {
            functionListHTML += makePTMFHTML(beginNode, realfBase, instanceName);
        }

        newPMTFHash += instanceName;
        
        if (beginNode == endNode) break;

        beginNode = dr.readPointer(beginNode + 4);
        if (beginNode == 0) break;
        
        count++;
        if (count > 1000) break;
    }

    if (!foundSelected) {
        mainRight.innerHTML = "";
    }

    newPMTFHash = hashString(newPMTFHash);

    if (prevPTMFHash != newPMTFHash) {
        mainLeft.innerHTML = functionListHTML;
    }

    prevPTMFHash = newPMTFHash;
}

function selectInstance(address) {
    let html = `
    <h2 id="funcAt">Instance at 0x${address.toString(16)}</h2>
    `

    let namePtr = dr.readPointer(address + 0x6C);
    let instanceName = arrayToString(dr.readValues(namePtr, 32));
    let inheritsFrom = inheritanceList[instanceName];
    if (inheritsFrom == undefined) inheritsFrom = "dBase";

    let inspectFuncs = [];

    let currInh = inheritsFrom;
    while (currInh) {
        if (inspectList[currInh]) {
            inspectFuncs.unshift({title: currInh}, ...inspectList[currInh]);
        }
        currInh = inheritanceList[currInh];
    };
    
    if (!!inspectList[instanceName]) {
        inspectFuncs.push({title: instanceName}, ...inspectList[instanceName]);
    }

    currentInspectFunc = inspectFuncs;

    let indicesEditable = [];

    for (let d = 0; d < currentInspectFunc.length; d++) {
        let display = currentInspectFunc[d];
        if (display.title) {
            html += `<h3>${display.title}</h3>`;
            currentInspectFunc.splice(d, 1);
            d--;
            continue;
        }
        html += `<div class="funcInfo" data-num="${d}"><span class="title">${display.name}</span>`;

        let editable = (!!display.change) ? "editable" : "";
        switch (display.type) {
            case "textbox":
                html += "<div>";
                html += `<span tabindex="0" class="values ${editable}"></span>`.repeat(display.count);
                html += "</div>";
                break;
            case "vec3":
            case "vec3s":
                html += `
                    <span tabindex="0" class="values x ${editable}"></span>
                    <span tabindex="0" class="values y ${editable}"></span>
                    <span tabindex="0" class="values z ${editable}"></span>`;
                break;
            case "flags":
                if (!display.flagNames) break;
                let isDisabled = (!display.change) ? "disabled" : "";
                html += "<div>";
                for (let flag of display.flagNames) {
                    html += `<div class="checkboxContainer">
                        <input type="checkbox" class="values ${editable}" ${isDisabled}>
                        <span class="checkboxLabel">${flag}</span>
                    </div>`;
                }
                html += "</div>";
                break;
            case "hexdump":
                html += `<span tabindex="0" class="values hexdump ${editable}"></span>`;
                break;
        }
        html += "</div>";
        indicesEditable[d] = (!!display.change);
    }

    mainRight.innerHTML = html;
    
    let containers = document.querySelectorAll(".funcInfo .values");

    for (let cont of containers) {
        let index = parseInt(cont.closest(".funcInfo").dataset.num);
        if (!indicesEditable[index]) continue;
        let editFunction = currentInspectFunc[index].change;
        if (cont.type == "checkbox") {
            cont.addEventListener("click", event => {
                editFunction(address, getValues(event.target));
            });
        } else {
            cont.addEventListener("focusin", event => {
                event.target.classList.add("nochange");
            
                if (event.target.classList.contains("editable")) {
                    event.target.setAttribute("contentEditable", "");
                }
            }, true);
            
            cont.addEventListener("focusout", event => {
                event.target.classList.remove("nochange");
            
                if (event.target.classList.contains("editable")) {
                    editFunction(address, getValues(event.target));
                    event.target.removeAttribute("contentEditable", "");
                }
            }, true);
    
            if (cont.classList.contains("editable")) {
                cont.addEventListener("keydown", event => {
                    if (event.keyCode != 13) return;
                    event.preventDefault();
        
                    editFunction(address, getValues(event.target));
        
                    event.target.blur();
                });
            }
        }
    }

    updateInstance(address);
}

function getValues(domEl) {
    let res = [];
    let valueEls = Array.from(domEl.closest(".funcInfo").getElementsByClassName("values"));
    if (domEl.type == "checkbox") {
        valueEls.forEach(el => {
            res.push(el.checked == true);
        });
    } else {
        valueEls.forEach(el => {
            res.push(el.textContent);
        });
    }
    return res;
}

function updateInstance(address) {
    let containers = document.querySelectorAll(".funcInfo");

    let stuff = [];
    
    for (let display of currentInspectFunc) {
        /*switch (display.type) {
            case "textbox":
            case "string":
                stuff.push(display.get(address));
                break;
            case "int":
                stuff.push(display.get(address).toString());
                break;
            case "float":
                stuff.push(display.get(address).toFixed(3));
                break;
            case "pointer":
                stuff.push("0x" + display.get(address).toString(16));
                break;
            case "flags":
                stuff.push(display.get(address));
                break;
            case "hexdump":
                stuff.push(Buffer.from(display.get(address)).toString("hex").replace(/(.{16})/g, "$1\n"));
                break;
        }*/
        stuff.push(display.get(address));
    }

    for (let i = 0; i < containers.length; i++) {
        if (stuff[i] == undefined) break;
        let vals = containers[i].querySelectorAll(".values");
        if (Array.isArray(stuff[i])) {
            for (let j = 0; j < vals.length; j++) {
                if (!vals[j].classList.contains("nochange")) {
                    if (vals[j].nodeName == "INPUT") {
                        vals[j].checked = stuff[i][j] == true;
                    } else {
                        if (vals[j].innerHTML != stuff[i][j]) {
                            vals[j].innerHTML = stuff[i][j];
                        }
                    }
                }
            }
        } else {
            if (!vals[0].classList.contains("nochange") && vals[0].innerHTML != stuff[i]) {
                vals[0].innerHTML = stuff[i];
            }
        }
    }

    /*let hexDumpText = "";
    for (let i = 0; i < 0x100; i += 8) {
        hexDumpText += buffer.subarray(i, i + 8).toString("hex") + "\n";
    }
    stuff.push(hexDumpText);**/
}

function removePTMF(address) {
    let thisPtr = dr.readPointer(address + 8);
    dr.writeU8(thisPtr + 0xB, 1); // set wasDestroyed to 1
}

makePTMFHTML = (ptmfaddr, addr, name) => `
<div class="function" data-ptmf="${ptmfaddr.toString(16)}">
    <span class="functionAddr">${addr.toString(16)}</span>
    <span class="actorName">${name}</span>
    <span class="PTMFRemove">❌</span>
</div>
`;

makePTMFHTMLSelected = (ptmfaddr, addr, name) => `
<div class="function selected" data-ptmf="${ptmfaddr.toString(16)}">
    <span class="functionAddr">${addr.toString(16)}</span>
    <span class="actorName">${name}</span>
    <span class="PTMFRemove">❌</span>
</div>
`;

function hashString(string) {     
    let hash = 0;
    if (string.length == 0) return hash;

    for (i = 0; i < string.length; i++) {
        char = string.charCodeAt(i);
        hash = ((hash << 5) - hash) + char;
        hash = hash & hash;
    }

    return hash; 
} 

function startWorking() {
    setInterval(readMemory, 50);
}

function arrayToString(arr) {
    let str = "";
    let i = 0;
    while (arr[i] != 0 && i < arr.length) {
        str += String.fromCharCode(arr[i]);
        i++;
    }
    return str;
}

function arrayToU32(arr) {
    return Buffer.from(arr).readUInt32BE();
}
