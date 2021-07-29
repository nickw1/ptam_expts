const canvas1 = document.getElementById('canvas1');
const ctx = canvas1.getContext('2d');
const video1 = document.getElementById('video1');
const width = 400, height = 300;
const FPS = 30;
let wasmPassTime, wasmPassLast = 0;
let active = false;
let ptr;
let pollHandle = null;

navigator.mediaDevices.getUserMedia({video: true}).then (stream => {
    video1.srcObject = stream;
    video1.play();
    setTimeout(processVideo, 500);    
    document.getElementById("start").addEventListener("click", e=> {
        active = true;
        document.getElementById("start").setAttribute("disabled", true);
        document.getElementById("stop").removeAttribute("disabled");
        pollHandle = setInterval(pollPTAM, 1000);
    });
    document.getElementById("stop").addEventListener("click", e=> {
        active = false;
        document.getElementById("start").removeAttribute("disabled");
        document.getElementById("stop").setAttribute("disabled", true);
        clearInterval(pollHandle);
        pollHandle = null;
    });
    document.getElementById("cleanup").addEventListener("click", e=> {
        Module._cleanup();
        Module._free(ptr);
        console.log('memory freed');
    });
});

function processVideo() {
    const begin = Date.now();
    ctx.drawImage(video1, 0, 0, width, height);
    wasmPassTime = Date.now();
    if(active && wasmPassTime - wasmPassLast > 2000) {
        sendCanvas(document.getElementById('canvas1'));
        wasmPassLast = wasmPassTime;
    }
    const delay = 1000/FPS - (Date.now() - begin);
    setTimeout(processVideo, delay);
}

function sendCanvas(canvas) {
    const ctx = canvas.getContext('2d');
    const imgData = ctx.getImageData(0, 0, width, height);
    const uint8ArrData = new Uint8Array(imgData.data);
    passToWasm(uint8ArrData, width, height);
}

function passToWasm(data, width, height) {
    console.log('passToWasm()');
    if(!ptr) {    
        ptr = Module._malloc(data.length * data.BYTES_PER_ELEMENT);
    }
    Module.HEAPU8.set(data, ptr);
    try {
//        Module._receiveData(ptr, width, height);
    } catch(e) { 
       console.log(e); 
    }
}

function pollPTAM() {
    const mapPoints = Module.getMapPoints();
    const poseMatrix = Module.getPoseMatrix();
}
