/*
  Rui Santos
  Complete project details at https://RandomNerdTutorials.com/esp32-mpu-6050-web-server/

  Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files.
  The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.
*/


let scene, camera, rendered, cube;

function degrees_to_radians(degrees)
{
  var pi = Math.PI;
  return degrees * (pi/180);
}

const ail_kpi_slider = document.querySelector("#ail_kpi_slider");  // Slider 
const ail_kpi_value  = document.querySelector("#ail_kpi_label");   // Anzeige
ail_kpi_value.textContent = ail_kpi_slider.value;

ail_kpi_slider.addEventListener("input", (event) => {
  ail_kpi_value.textContent = concat("Ki=", event.target.value);
 
  const msg = {
    action: "PIDChanged",
    Kpi: event.target.value,   
    date: Date.now(),
  };

  socket.send(JSON.stringify(msg));
});


const socket = new WebSocket('ws://' + location.host + '/echo');
      socket.addEventListener('message', ev => {
        console.log(ev.data);
        let MPU = JSON.parse(ev.data);   

        ServoLeft.rotation.x = degrees_to_radians(MPU["ServoLeft"]);
        ServoLeft.rotation.y = 0;
        ServoLeft.rotation.z = 0;

        ServoRight.rotation.x = degrees_to_radians(MPU["ServoRight"]);
        ServoRight.rotation.y = 0;
        ServoRight.rotation.z = 0;

        Plane.rotation.z = degrees_to_radians(MPU["roll"]);

        renderer.render(scene, camera);  
        document.getElementById("yaw").innerHTML = MPU["yaw"];
        document.getElementById("pitch").innerHTML = MPU["pitch"];
        document.getElementById("roll").innerHTML = MPU["roll"];    
        document.getElementById("ServoLeft").innerHTML = MPU["ServoLeft"];
        document.getElementById("ServoRight").innerHTML = MPU["ServoRight"];

      });

function parentWidth(elem) {
  return elem.parentElement.clientWidth;
}

function parentHeight(elem) {
  return elem.parentElement.clientHeight;
}

function init3D(){

  scene = new THREE.Scene();
  scene.background = new THREE.Color(0xffffff);

  camera = new THREE.PerspectiveCamera(45, parentWidth(document.getElementById("3Dcube")) / parentHeight(document.getElementById("3Dcube")), 1, 500);

  renderer = new THREE.WebGLRenderer({ antialias: true });
  renderer.setSize(parentWidth(document.getElementById("3Dcube")), parentHeight(document.getElementById("3Dcube")));

  document.getElementById('3Dcube').appendChild(renderer.domElement);

  const geometry = new THREE.BoxGeometry( 4, 0.1, 1 );  
  const material = new THREE.MeshBasicMaterial( { color: 0x0000ff } );
  ServoLeft   = new THREE.Mesh( geometry, material );  
  ServoRight  = new THREE.Mesh( geometry, material );  
  scene.add( ServoLeft );
  scene.add( ServoRight );

  const geometryPlane = new THREE.BoxGeometry( 8, 1, 2 );  
  const materialPlane = new THREE.MeshBasicMaterial( { color: 0x00ff00 } );
  Plane   = new THREE.Mesh( geometryPlane, materialPlane );  
  scene.add( Plane );

  ServoLeft.position.x = -10;
  ServoRight.position.x = 10;

  //const box2 = new THREE.Box3();
  //box2.setFromCenterAndSize( new THREE.Vector3( 1, 1, 1 ), new THREE.Vector3( 2, 1, 3 ) );
  //const helper = new THREE.Box3Helper( box2, 0xff8000 );
  //scene.add( helper );

  const gridHelper = new THREE.GridHelper( 10, 10, 0xff0000 );
  scene.add( gridHelper );  
  
  camera.position.x = 0;
  camera.position.y = 5;
  camera.position.z = 7;  
  camera.lookAt( 0, 0, 0 );

  renderer.render(scene, camera);
}

// Resize the 3D object when the browser window changes size
function onWindowResize(){
  camera.aspect = parentWidth(document.getElementById("3Dcube")) / parentHeight(document.getElementById("3Dcube"));
  //camera.aspect = window.innerWidth /  window.innerHeight;
  camera.updateProjectionMatrix();
  //renderer.setSize(window.innerWidth, window.innerHeight);
  renderer.setSize(parentWidth(document.getElementById("3Dcube")), parentHeight(document.getElementById("3Dcube")));

}

window.addEventListener('resize', onWindowResize, false);

// Create the 3D representation
init3D();


function resetPosition(element){
  init3D();
  var xhr = new XMLHttpRequest();
  xhr.open("GET", "/"+element.id, true);
  console.log(element.id);
  xhr.send();
}

function setPosition(element){
  // Change cube rotation after receiving the readinds
  cube.rotation.x += 1;
  cube.rotation.y += 1;
  cube.rotation.z += 1;
  renderer.render(scene, camera);  
}
