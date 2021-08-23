import * as THREE from './node_modules/three/build/three.module.js';

let scene, camera, renderer, canvas1, camera2, scene2, video, wasmPassTime, wasmPassLast = 0, active = false, nKeyFrames = 0, ptr, pollHandle = null, poseMatrix= null, invisibleCanvas, ctx, imgData2, mapPointGeom, mapPointMtl, objects = [];

const camWidth = 480, camHeight = 640, FPS = 30;

init();
animate();

function init() {
    scene = new THREE.Scene();
    camera = new THREE.PerspectiveCamera(80, camWidth/camHeight, 0.00001, 1);
    canvas1 = document.getElementById("canvas1");
    renderer = new THREE.WebGLRenderer({canvas: canvas1});
    renderer.autoClear = false;

    scene2 = new THREE.Scene();
    camera2 = new THREE.OrthographicCamera(-0.5, 0.5, 0.5, -0.5, 0, 0.1);


    window.addEventListener("resize", onResize);

    video = document.createElement("video");
    video.setAttribute("autoplay", true);
    video.setAttribute("playsinline", true);

    invisibleCanvas = document.createElement("canvas");
    invisibleCanvas.width = camWidth;
    invisibleCanvas.height = camHeight;
    ctx = invisibleCanvas.getContext('2d');

    navigator.mediaDevices.getUserMedia({
        video:{
            facingMode: 'environment', 
            width: camWidth, 
            height: camHeight
        }
    }).then(feed => {
        video.srcObject = feed;
        setTimeout(processVideo, 500);
    });

    const geom = new THREE.PlaneBufferGeometry();
    const mtl = new THREE.MeshBasicMaterial({map: new THREE.VideoTexture(video)}); 
    const mesh = new THREE.Mesh(geom, mtl);
    scene2.add(mesh);

    //imgData2 = new Uint8Array(camWidth * camHeight * 4);

    mapPointGeom = new THREE.BoxGeometry(0.0001, 0.0001, 0.0001);
    mapPointMtl = new THREE.MeshBasicMaterial({color:0xff0000});

    /*
    for(let  i=-20; i<20; i++) {
        const mesh1 = new THREE.Mesh(mapPointGeom, mapPointMtl);
        mesh1.position.x = 0.001 * i;
        mesh1.position.z = (-0.001 * i) - 0.001;
        mesh1.position.y = 0;
        scene.add(mesh1);
    }
    */

    setupUI();
    onResize();
}

function animate() {
    renderer.render(scene2, camera2);
    const now = Date.now();
    renderer.clearDepth();
    renderer.render(scene, camera);
    requestAnimationFrame(animate);
}

function onResize() {
    renderer.setSize(canvas1.clientWidth, canvas1.clientHeight);
    camera.aspect = canvas1.clientWidth / canvas1.clientHeight;
    camera.updateProjectionMatrix();
}

function setupUI() {
    document.getElementById("start").addEventListener("click", e=> {
        document.getElementById("start").setAttribute("disabled", true);
        if(nKeyFrames === 0) {
            status('Press the button to capture the first key frame.');
            document.getElementById("captureKeyFrame").removeAttribute("disabled");
        } else {
            status("Started.");
        }
        document.getElementById("stop").removeAttribute("disabled");
        active = true;
        pollHandle = setInterval(pollPTAM, 5000);
    });
    document.getElementById("captureKeyFrame").addEventListener("click", e=> {
        const ret = Module._captureKeyFrame();    
        if(++nKeyFrames == 1) {
            status('Press the button again to capture the next keyframe.');
            document.getElementById("captureKeyFrame").value = 'Capture key frame 2';
        } else {
            document.getElementById("captureKeyFrame").setAttribute("disabled", true);
        }
    });
    document.getElementById("stop").addEventListener("click", e=> {
        active = false;
        document.getElementById("start").removeAttribute("disabled");
        document.getElementById("stop").setAttribute("disabled", true);
        status("Stopped.");
        clearInterval(pollHandle);
        pollHandle = null;
    });
    document.getElementById("cleanup").addEventListener("click", e=> {
        Module._cleanup();
        Module._free(ptr);
        status("Memory freed.");
        document.getElementById("start").setAttribute("disabled", true);
        document.getElementById("captureKeyFrame").setAttribute("disabled", true);
        document.getElementById("stop").setAttribute("disabled", true);
        document.getElementById("cleanup").setAttribute("disabled", true);
        console.log('memory freed');
    });
}

function processVideo() {
    ctx.drawImage(video, 0, 0, camWidth, camHeight);
    wasmPassTime = Date.now();
    if(active && wasmPassTime - wasmPassLast > 1000) {
        sendCanvas(invisibleCanvas);
        wasmPassLast = wasmPassTime;
    }
//    const delay = 1000/FPS - (Date.now() - wasmPassTime);
    const delay = 500;
    setTimeout(processVideo, delay);
}

function sendCanvas(canvas) {
    const ctx = canvas.getContext('2d');
    const imgData = ctx.getImageData(0, 0, camWidth, camHeight);
    // attempt to read webgl canvas directly - gives blank data
    //const ctx2 = canvas1.getContext('webgl2');
    //ctx2.readPixels(0, 0, camWidth, camHeight, ctx2.RGBA, ctx2.UNSIGNED_BYTE, imgData2, 0);
    const uint8ArrData = new Uint8Array(imgData.data);
    passToWasm(uint8ArrData, camWidth, camHeight);
}

function passToWasm(data, width, height) {
    if(!ptr) {    
        ptr = Module._malloc(data.length * data.BYTES_PER_ELEMENT);
    }
    Module.HEAPU8.set(data, ptr);
    try {
        const trackStatus = Module._receiveData(ptr, width, height);
        if(trackStatus == 0) {
            nKeyFrames = 0;
            document.getElementById("captureKeyFrame").removeAttribute("disabled");
            document.getElementById("captureKeyFrame").value = 'Capture key frame 1';
            status('Could not track - resetting, please capture first key frame again.');
            alert('Could not track - resetting, please capture first key frame again.');
        } else if (trackStatus == 2) {
            alert('Both keyframes captured, tracking...');
            status('Both keyframes captured, tracking...');
        }
    } catch(e) { 
       console.log(e); 
    }
}

function pollPTAM() {
    if(poseMatrix === null) {
        poseMatrix = new Module.PoseMatrix();
    }
    const mapPoints = Module.getMapPoints();
    try {
        console.log(`JS: mapPoints size ${mapPoints.size()}`)
        for(let i=0; i<mapPoints.size(); i+=3) {
            console.log(`${i} ${mapPoints.get(i)} ${mapPoints.get(i+1)} ${mapPoints.get(i+2)}`);
        }
    } catch(e) {
        console.error(e);
    }
    poseMatrix.loadLatestMatrix();
    
    
    console.log('JS: poseMatrix:')
    let str = "";
    for(let i=0; i<16; i++) {
        if(!(i%4)) {
            str += "\n";
        }
        str += `${poseMatrix.get(i)} `;

    }
    //addVisibleMapPoints(poseMatrix, mapPoints);
    console.log(str);
}

function addVisibleMapPoints(poseMatrix, mapPoints) {
    for(let object of objects) {
        scene.remove(object);
    }
    objects = [];
    const m = new Array(16);
    for(let i=0; i<16; i++) {
        m[i] = poseMatrix.get(i);
    }
    camera.matrixWorldInverse = m;
    for(let i=0; i<mapPoints.size(); i+=3)  {
        const mesh = new THREE.Mesh(mapPointGeom, mapPointMtl);
        mesh.translation.x = mapPoints.get(i);
        mesh.translation.y = mapPoints.get(i+1);
        mesh.translation.z = mapPoints.get(i+2);
        scene.add(mesh);
        objects.push(mesh);
    }
}

function status(msg) {
    document.getElementById('status').innerHTML = msg;
}
