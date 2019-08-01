var xyzzyStatus = "0";
var xyzzyName;
var xyzzyMatrix;
// not started = "0"
// Waiting for name = "1"
// Playing = "2"
// Playing WS = "3"
// gameover = "4"

//Start
var soundStart = document.createElement('audio');
soundStart.setAttribute('src', 'img/xyzzy/start.wav');

//success
var soundSuccess = document.createElement('audio');
soundSuccess.setAttribute('src', 'img/xyzzy/success.wav');

//move
var soundMove = document.createElement('audio');
soundMove.setAttribute('src', 'img/xyzzy/move.wav');

//drop
var soundDrop = document.createElement('audio');
soundDrop.setAttribute('src', 'img/xyzzy/drop.mp3');
//drop
var soundSpin = document.createElement('audio');
soundSpin.setAttribute('src', 'img/xyzzy/Spin.wav');

//GameOver
var soundGameOver = document.createElement('audio');
soundGameOver.setAttribute('src', 'img/xyzzy/gameover.wav');

function openXyzzy() {
  //CHECK FOR ALREADY STARTED GAME!!
  $('#xyzzy').modal('show');
  xyzzyDraw("step1");
  xyzzyStatus = "1";

  //load highscore
  $('#highscore').html("High Score: " + xyzzyHighScore.highscore);
  $('#highscoreName').html(jsUcfirst(xyzzyHighScore.highscoreplayer));

  //Populate Last Name
  if (xyzzyName != undefined) {
    $('#xyzzyName')[0].value = xyzzyName;
  }

  //load matrices
  for (var i = 0; i < availableMatrices.matrices.length; i++) {
    $('#matrixList').append("<option value='" + availableMatrices.matrices[i] + "'>" + availableMatrices.matrices[i] + "</option>");
  }
  //puploate Last Matrix
  if (xyzzyMatrix != undefined) {
    console.log("Loaded Last Matrix Setting");
    $('#matrixList')[0].value = xyzzyMatrix;
  }
}

function startXyzzy() {
  $('body').addClass('disableSelection');
  // if mobile {
  //   $('body').addClass('disableSelection');
  // }
  xyzzyName = $('#xyzzyName')[0].value;
  xyzzyMatrix = $('#matrixList')[0].value;

  if (xyzzyName == '') {
    //Default Name
    xyzzyName = "Donald Trump";
  }

  xyzzyCommand('initialise', xyzzyMatrix + ',' + xyzzyName);
  xyzzyCommand('g');

  xyzzyDraw("step2");
  $("#xyzzyPlayerName").html(xyzzyName);

  //check is sockets are running
  if (socket.readyState > '1') {
    xyzzyStatus = '2';
    xyzzyStatusAJAX();
  } else {
    xyzzyStatus = '3';
  }
}

function xyzzyRestart() {
  xyzzyCommand('close');
  openXyzzy();
  xyzzyStatus = "1";
}

//websocket listener
socket.addEventListener('message', function(event) {
  var response = JSON.parse(event.data);
  if (xyzzyStatus == '3') {
    updateXyzzy(response);
  }
  if (response.xyzzyevent != undefined){

    if (response.xyzzyevent == 'initialised'){
      soundStart.play();
    }

    if (response.xyzzyevent == 'rowcomplete'){
      soundSuccess.play();
    }
    if (response.xyzzyevent == 'movedleft' || response.xyzzyevent == 'movedright' || response.xyzzyevent == 'piecelanded'){
      soundMove.play();
    }
    if (response.xyzzyevent == 'dropped'){
      soundDrop.play();
    }
    if (response.xyzzyevent == 'spun'){
      soundSpin.play();
    }

  }
});


function xyzzyStatusAJAX() {

  var timer = window.setInterval(function() {
    $.ajax({
      url: '/xyzzy?c=q',
      success: function(response) {
        //console.log(response);
        updateXyzzy(response);
      }
    })

  }, 1000);

}

function updateXyzzy(response) {
  if (response.result == 'running') {
    //update Score
    //update next peice
    console.log(response.next);
    $('#xyzzyNext').attr('src', 'img/xyzzy/'+response.next+'.png');
    $("#xyzzyCurrentScore").html("Your Score: " + response.score);

  } else if (response.result == 'gameover') {
    xyzzyStatus = "4";
    xyzzyDraw("step3");
    $("#xyzzyPlayerName").html(jsUcfirst(response.playername));
    $("#xyzzyScore").html("Your Score: " + response.score);
    //console.log(response.score);
    $("#xyzzyHighScoreName").html(jsUcfirst(response.highscoreplayer));
    $("#xyzzyHighScore").html("High Score: " + response.highscore);

    //update high score
    xyzzyHighScore = JSON.parse('{"highscoreplayer":"' + response.highscoreplayer + '","highscore":' + response.highscore + '}');
  }
  // stop if game closed
  // if ($('body').hasClass("modal-open")) {
  //   xyzzyCommand('close');
  //   xyzzyStatus = "0";
  //   clearInterval(timer);
  // }
}

function xyzzyCommand(c, p) {
  if (socket.readyState <= '1') {
    if (p != undefined) {
      socket.send('{"Type":"xyzzy","c":"' + c + '","p":"' + p + '"}');
    } else {
      socket.send('{"Type":"xyzzy","c":"' + c + '"}');
    }
  } else {
    if (p != undefined) {
      $.ajax({
        url: '/xyzzy?c=' + c,
        success: function(response) {}
      });
    } else {
      $.ajax({
        url: '/xyzzy?c=' + c + '&p=' + p,
        success: function(response) {}
      });
    }
  }
}


function xyzzyDraw(step) {
  var step1 =
    `<h2 id="highscoreName"></h2>
    <h4 id="highscore"></h4><hr>
    <label>Select Matrix</label>
    <select class="form-control" id="matrixList">
    </select>
    <br />
    <lable>Enter Name:</lable>
    <input id="xyzzyName" type="text" class="form-control"> <br />
    <button data-dismiss="modal" aria-label="Close" onclick="startXyzzy($('#xyzzyName').value)" class="btn btn-success">Play</button>`;

  var step2 =
    `<h2 id="xyzzyPlayerName"></h2>
    <h4 id="xyzzyCurrentScore">Your Score: 320</h4>
    <div class="col-md-3>
      <a href="#" class="thumbnail"><img id="xyzzyNext" src="img/xyzzy/I.png"></a>
    </div>
    <br/>
    <button onclick="xyzzyCommand('s')" class="btn btn-default glyphicon glyphicon-arrow-up btn-xyzzy"></button>
    <br />
    <button onclick="xyzzyCommand('l')" class="btn btn-default glyphicon glyphicon-arrow-left btn-xyzzy"></button>
    <button onclick="xyzzyCommand('d')" class="btn btn-default glyphicon glyphicon-arrow-down btn-xyzzy"></button>
    <button onclick="xyzzyCommand('r')" class="btn btn-default glyphicon glyphicon-arrow-right btn-xyzzy"></button>
    <br /><br />
    <button onclick="xyzzyCommand('c'); xyzzyCommand('g')" class="btn btn-default btn-warning">Reset</button>
    <button onclick="xyzzyCommand('c')" class="btn btn-default btn-danger">Quit</button>`;

  var step3 =
    `<h1>Game Over</h1>
    <h2 id="xyzzyPlayerName"></h2>
    <h4 id="xyzzyScore"></h4>
    <h2 id="xyzzyHighScoreName"></h2>
    <h4 id="xyzzyHighScore"></h4>
    <br />
    <button onclick="xyzzyRestart()" class="btn btn-info">Restart</button>`;

  if (step == "step1") {
    $("#xyzzyContainer").html(step1);
  } else if (step == "step2") {
    $("#xyzzyContainer").html(step2);
  } else if (step == "step3") {
    soundGameOver.play();
    $("#xyzzyContainer").html(step3);
  }
}

// konami

var Konami = function(callback) {
  var konami = {
    addEvent: function(obj, type, fn, ref_obj) {
      if (obj.addEventListener)
        obj.addEventListener(type, fn, false);
      else if (obj.attachEvent) {
        // IE
        obj["e" + type + fn] = fn;
        obj[type + fn] = function() {
          obj["e" + type + fn](window.event, ref_obj);
        }
        obj.attachEvent("on" + type, obj[type + fn]);
      }
    },
    input: "",
    pattern: "38384040373937396665",
    load: function(link) {
      this.addEvent(document, "keydown", function(e, ref_obj) {
        if (ref_obj) konami = ref_obj; // IE
        konami.input += e ? e.keyCode : event.keyCode;
        if (konami.input.length > konami.pattern.length)
          konami.input = konami.input.substr((konami.input.length -
            konami.pattern.length));
        if (konami.input == konami.pattern) {
          konami.code(link);
          konami.input = "";
          e.preventDefault();
          return false;
        }
      }, this);
      this.iphone.load(link);
    },
    code: function(link) {
      window.location = link
    },
    iphone: {
      start_x: 0,
      start_y: 0,
      stop_x: 0,
      stop_y: 0,
      tap: false,
      capture: false,
      orig_keys: "",
      keys: ["UP", "UP", "DOWN", "DOWN", "LEFT", "RIGHT", "LEFT", "RIGHT",
        "TAP", "TAP"
      ],
      code: function(link) {
        konami.code(link);
      },
      load: function(link) {
        this.orig_keys = this.keys;
        konami.addEvent(document, "touchmove", function(e) {
          if (e.touches.length == 1 && konami.iphone.capture == true) {
            var touch = e.touches[0];
            konami.iphone.stop_x = touch.pageX;
            konami.iphone.stop_y = touch.pageY;
            konami.iphone.tap = false;
            konami.iphone.capture = false;
            konami.iphone.check_direction();
          }
        });
        konami.addEvent(document, "touchend", function(evt) {
          if (konami.iphone.tap == true) konami.iphone.check_direction(
            link);
        }, false);
        konami.addEvent(document, "touchstart", function(evt) {
          konami.iphone.start_x = evt.changedTouches[0].pageX;
          konami.iphone.start_y = evt.changedTouches[0].pageY;
          konami.iphone.tap = true;
          konami.iphone.capture = true;
        });
      },
      check_direction: function(link) {
        x_magnitude = Math.abs(this.start_x - this.stop_x);
        y_magnitude = Math.abs(this.start_y - this.stop_y);
        x = ((this.start_x - this.stop_x) < 0) ? "RIGHT" : "LEFT";
        y = ((this.start_y - this.stop_y) < 0) ? "DOWN" : "UP";
        result = (x_magnitude > y_magnitude) ? x : y;
        result = (this.tap == true) ? "TAP" : result;

        if (result == this.keys[0]) this.keys = this.keys.slice(1, this.keys
          .length);
        if (this.keys.length == 0) {
          this.keys = this.orig_keys;
          this.code(link);
        }
      }
    }
  }

  typeof callback === "string" && konami.load(callback);
  if (typeof callback === "function") {
    konami.code = callback;
    konami.load();
  }

  return konami;
};
