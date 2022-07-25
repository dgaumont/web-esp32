var display;
var score;
var socket;
var connected = false;
var sounds=[];
var aid = null;

function animate() {
    var value = (score).toPrecision(8).split('.').reverse().join('');
    //console.log("#### value="+value);
    //score += 1;
    display.setValue(value);
    if (connected == true)
        socket.send("get_score");
    window.setTimeout('animate()', 500);
}

function initWebSocket() {
    //socket = new WebSocket("ws://127.0.0.1:5678/");

    //socket = new WebSocket("ws://192.168.1.1/ws"); // AP mode
    socket = new WebSocket("ws://192.168.1.25/ws"); // WIFI mode

    socket.onopen = function(e) {
        console.log("[open] Connection established");
        socket.send("get_score");
    };

    socket.onmessage = function(event) {
        console.log(`[message] Data received from server: ${event.data}`);
        const pinball_info = event.data.split(" ");
        score = Number(pinball_info[0]);
        sound_id = Number(pinball_info[1]);
        animation_id = Number(pinball_info[2]);
        console.log('[message] '+score+' '+animation_id+' '+sound_id);
        connected = true;
        if (sound_id > 0) {
            playSound(sound_id);
        }
        if (animation_id > 0) {
            play_animation1();
        }
    };

    socket.onclose = function(event) {
        if (event.wasClean) {
            console.log(`[close] Connection closed cleanly, code=${event.code} reason=${event.reason}`);
        } else {
            // e.g. server process killed or network down
            // event.code is usually 1006 in this case
            alert('[close] Connection died');
        }
    };

    socket.onerror = function(error) {
        alert(`[error] ${error.message}`);
    };
}

function playSound(id) {
  sounds[id].play();
} 

function startPinballDisplay() {
    display = new SegmentDisplay("display");
    display.pattern         = "##:##:##";
    display.cornerType      = 2;
    display.displayType     = 7;
    display.displayAngle    = 9;
    display.digitHeight     = 20;
    display.digitWidth      = 12;
    display.digitDistance   = 2;
    display.segmentWidth    = 3;
    display.segmentDistance = 0.5;
    display.colorOn         = "rgba(0, 0, 0, 0.9)";
    display.colorOff        = "rgba(0, 0, 0, 0.1)";

    score = 0;
    initWebSocket();
    animate();
}

function play_animation1() {
  var elem = document.getElementById("animation_1");
  var pos = 0;
  clearInterval(aid);
  aid = setInterval(frame, 0.5);
  function frame() {
    if (pos == 350) {
      clearInterval(aid);
    } else {
      pos++; 
      elem.style.left = pos + 'px'; 
    }
  }
}

function initPinball() {
    // sounds setup
    sounds.push(document.getElementById("buzz_1"));
    sounds.push(document.getElementById("buzz_2"));
    sounds.push(document.getElementById("bip"));
    sounds.push(document.getElementById("terminator"));

    startPinballDisplay();
}


