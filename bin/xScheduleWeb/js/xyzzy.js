var xyzzyStatus = "";
var xyzzyName;
// not started = "0"
// waiting for name = "1"
// playing = "2"
// gameover = "3"

$(document).keydown(function(e) {
  if (e.keyCode == 37) {
    console.log("left pressed");
    return false;
  }
}), 1000;

function openXyzzy() {
  $('#xyzzy').modal('show');
  xyzzyDraw("step1");
  xyzzyStatus = "1";
  //get high score
  socket.send('{"Type":"xyzzy","c":"initialise"}');
  sleep(1).then(() => {
    socket.send('{"Type":"query","Query":"GetMatrices","Reference":"loadMatrix"}');
  });


  //xyzzyCommand('initialise');
  // $('#xyzzySelectionDisable').disableSelection();
}

function startXyzzy() {
  var xyzzyName = $('#xyzzyName')[0].value;
  var matrixList = $('#matrixList')[0].value;
  if (xyzzyName == '') {
    xyzzyName = "Donald Trump";
  }
  //var xyzzyMatrix = $('#xyzzyMatrix')[0].value;
  xyzzyCommand('initialise&p=' + matrixList + ',' + xyzzyName);
  xyzzyCommand('g');
  xyzzyStatusTimer();
  xyzzyDraw("step2");
  xyzzyStatus = "2";
  $("#xyzzyPlayerName").html(xyzzyName);

}

function xyzzyRestart() {
  xyzzyCommand('close');
  xyzzyDraw("step1");
  xyzzyStatus = "1";
  socket.send('{"Type":"xyzzy","c":"initialise"}');
  sleep(2).then(() => {
    socket.send('{"Type":"query","Query":"GetMatrices","Reference":"loadMatrix"}');
  });
}

function xyzzyReset() {
  xyzzyCommand('c');
  xyzzyCommand('g');
}

//websocket listener
socket.addEventListener('message', function(event) {
  var response = JSON.parse(event.data);

  if (response.score != undefined || response.highscore != undefined) {
    if (xyzzyStatus == "1" & response.message == 'Two parameters expected ... name of matrix and players name.') {
      $('#highscore').html("High Score: " + response.highscore);
      $('#highscoreName').html(jsUcfirst(response.highscoreplayer));
    }
  }


});

function xyzzyStatusTimer() {

  var timer = window.setInterval(function() {
    $.ajax({
      url: '/xyzzy?c=q',
      success: function(response) {
        //console.log(response);

        if (response.result == 'running') {
          //update Score
          //update next peice
          $("#xyzzyNext").html(response.next);
          $("#xyzzyCurrentScore").html("Your Score: " + response.score);


        } else if (response.result == 'failed') {
          xyzzyStatus = "0";
          clearInterval(timer);
        } else if (response.result == 'gameover') {
          xyzzyStatus = "3";
          xyzzyDraw("step3");
          $("#xyzzyPlayerName").html(response.playername);
          $("#xyzzyScore").html("Your Score: " + response.score);
          //console.log(response.score);
          $("#xyzzyHighScoreName").html(response.highscoreplayer);
          $("#xyzzyHighScore").html("High Score: " + response.highscore);
        }

        //stop if game closed
        if ($('body').attr('class') != "modal-open") {
          xyzzyCommand('close');
          xyzzyStatus = "0";
          clearInterval(timer);
        }
      }
    })

  }, 1000);

}

function xyzzyCommand(c) {
  $.ajax({
    url: '/xyzzy?c=' + c,
    success: function(response) {
      //console.log("I win");
    }
  });
}

function loadMatrix(response) {
  console.log(response);
  for (var i = 0; i < response.matrices.length; i++) {
    $('#matrixList').append("<option value='" + response.matrices[i] + "'>" + response.matrices[i] + "</option>");
  }
}

function xyzzyDraw(step) {
  var step1 =
    `<h2 id="highscoreName"></h2>
  <h4 id="highscore"></h4>
  <label>Select Matrix</label>
<select class="form-control" id="matrixList">
</select>
  <lable>Enter Name:</lable>
  <input id="xyzzyName" type="text" class="form-control">
  <button data-dismiss="modal" aria-label="Close" onclick="startXyzzy($('#xyzzyName').value)" class="btn btn-success">Play</button>`;

  var step2 =
    `<h2 id="xyzzyPlayerName"></h2>
  <h4 id="xyzzyCurrentScore">Your Score: 320</h4>
  <button id="xyzzyNext" class="btn btn-default tetrisL btn-xyzzy"></button>
  <br/><br/><br/>
  <button onclick="xyzzyCommand('s')" class="btn btn-default glyphicon glyphicon-arrow-up btn-xyzzy"></button>
  <br />
  <button onclick="xyzzyCommand('l')" class="btn btn-default glyphicon glyphicon-arrow-left btn-xyzzy"></button>
  <button onclick="xyzzyCommand('d')" class="btn btn-default glyphicon glyphicon-arrow-down btn-xyzzy"></button>
  <button onclick="xyzzyCommand('r')" class="btn btn-default glyphicon glyphicon-arrow-right btn-xyzzy"></button>
  <br /><br />
  <button onclick="xyzzyReset()" class="btn btn-default btn-warning">Reset</button>
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
