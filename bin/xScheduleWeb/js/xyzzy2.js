var xyzzy2Status = "0";
var xyzzy2Name;
var xyzzy2Matrix;
var lastScore;
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

//spin
var soundSpin = document.createElement('audio');
soundSpin.setAttribute('src', 'img/xyzzy/Spin.wav');

//GameOver
var soundGameOver = document.createElement('audio');
soundGameOver.setAttribute('src', 'img/xyzzy/gameover.wav');

function openXyzzy2() {
  //CHECK FOR ALREADY STARTED GAME!!
  $('#xyzzy2').modal('show');
  xyzzy2Draw("step1");
  xyzzy2Status = "1";

  //load highscore
  $('#highscore').html("High Score: " + xyzzy2HighScore.highscore);
  $('#highscoreName').html(jsUcfirst(xyzzy2HighScore.highscoreplayer));

  //Populate Last Name
  if (xyzzy2Name != undefined) {
    $('#xyzzy2Name')[0].value = xyzzy2Name;
  }

  //load matrices
  for (var i = 0; i < availableMatrices.matrices.length; i++) {
    $('#matrix2List').append("<option value='" + availableMatrices.matrices[i] + "'>" + availableMatrices.matrices[i] + "</option>");
  }
  //puploate Last Matrix
  if (xyzzy2Matrix != undefined) {
    console.log("Loaded Last Matrix Setting");
    $('#matrix2List')[0].value = xyzzy2Matrix;
  }
}

function startXyzzy2() {
  $('body').addClass('disableSelection');
  // if mobile {
  //   $('body').addClass('disableSelection');
  // }
  xyzzy2Name = $('#xyzzy2Name')[0].value;
  xyzzy2Matrix = $('#matrix2List')[0].value;

  if (xyzzy2Name == '') {
    //Default Name
    xyzzy2Name = "Voldemort";
  }

  xyzzy2Command('initialise', xyzzy2Matrix + ',' + xyzzy2Name);
  xyzzy2Command('g');

  lastScore = 0;

  xyzzy2Draw("step2");
  $("#xyzzy2PlayerName").html(xyzzy2Name);

  //check is sockets are running
  if (socket.readyState > '1') {
    xyzzy2Status = '2';
    xyzzy2StatusAJAX();
  } else {
    xyzzy2Status = '3';
  }
}

function xyzzy2Restart() {
  xyzzy2Command('close');
  openXyzzy2();
  xyzzy2Status = "1";
  lastScore = 0;
}

//websocket listener
socket.addEventListener('message', function(event) {
  var response = JSON.parse(event.data);
  if (xyzzy2Status == '3') {
    updateXyzzy2(response);
  }
  if (response.xyzzyevent != undefined){

    if (response.xyzzyevent == 'initialised'){
      soundStart.play();
    }

    if (response.xyzzyevent == 'movingleft' || response.xyzzyevent == 'movingright' || response.xyzzyevent == 'movingup' || response.xyzzyevent == 'movingdown'){
      soundMove.play();
    }
  }
});


function xyzzy2StatusAJAX() {

  var timer = window.setInterval(function() {
    $.ajax({
      url: '/xyzzy2?c=q',
      success: function(response) {
        //console.log(response);
        updateXyzzy2(response);
      }
    })

  }, 1000);

}

function updateXyzzy2(response) {
  if (response.result == 'running') {
    //update Score
    $("#xyzzyCurrentScore").html("Your Score: " + response.score);

	if (response.score > lastScore)
	{
		lastScore = response.score;
		soundSuccess.play();
	}

  } else if (response.result == 'gameover') {
    xyzzy2Status = "4";
    xyzzy2Draw("step3");
    $("#xyzzy2PlayerName").html(jsUcfirst(response.playername));
    $("#xyzzy2Score").html("Your Score: " + response.score);
    //console.log(response.score);
    $("#xyzzy2HighScoreName").html(jsUcfirst(response.highscoreplayer));
    $("#xyzzy2HighScore").html("High Score: " + response.highscore);

    //update high score
    xyzzy2HighScore = JSON.parse('{"highscoreplayer":"' + response.highscoreplayer + '","highscore":' + response.highscore + '}');
  }
  // stop if game closed
  // if ($('body').hasClass("modal-open")) {
  //   xyzzyCommand('close');
  //   xyzzyStatus = "0";
  //   clearInterval(timer);
  // }
}

function xyzzy2Command(c, p) {
  if (socket.readyState <= '1') {
    if (p != undefined) {
      socket.send('{"Type":"xyzzy2","c":"' + c + '","p":"' + p + '"}');
    } else {
      socket.send('{"Type":"xyzzy2","c":"' + c + '"}');
    }
  } else {
    if (p != undefined) {
      $.ajax({
        url: '/xyzzy2?c=' + c,
        success: function(response) {}
      });
    } else {
      $.ajax({
        url: '/xyzzy2?c=' + c + '&p=' + p,
        success: function(response) {}
      });
    }
  }
}


function xyzzy2Draw(step) {
  var step1 =
    `<h2 id="highscoreName"></h2>
    <h4 id="highscore"></h4><hr>
    <label>Select Matrix</label>
    <select class="form-control" id="matrix2List">
    </select>
    <br />
    <lable>Enter Name:</lable>
    <input id="xyzzy2Name" type="text" class="form-control"> <br />
    <button data-dismiss="modal" aria-label="Close" onclick="startXyzzy2($('#xyzzy2Name').value)" class="btn btn-success">Play</button>`;

  var step2 =
    `<h2 id="xyzzy2PlayerName"></h2>
    <h4 id="xyzzy2CurrentScore">Your Score: 320</h4>
    <br/>
    <button onclick="xyzzy2Command('u')" class="btn btn-default glyphicon glyphicon-arrow-up btn-xyzzy"></button>
    <br />
    <button onclick="xyzzy2Command('l')" class="btn btn-default glyphicon glyphicon-arrow-left btn-xyzzy"></button>
    <button onclick="xyzzy2Command('d')" class="btn btn-default glyphicon glyphicon-arrow-down btn-xyzzy"></button>
    <button onclick="xyzzy2Command('r')" class="btn btn-default glyphicon glyphicon-arrow-right btn-xyzzy"></button>
    <br /><br />
    <button onclick="xyzzy2Command('c'); xyzzy2Command('g')" class="btn btn-default btn-warning">Reset</button>
    <button onclick="xyzzy2Command('c')" class="btn btn-default btn-danger">Quit</button>`;

  var step3 =
    `<h1>Game Over</h1>
    <h2 id="xyzzy2PlayerName"></h2>
    <h4 id="xyzzy2Score"></h4>
    <h2 id="xyzzy2HighScoreName"></h2>
    <h4 id="xyzzy2HighScore"></h4>
    <br />
    <button onclick="xyzzy2Restart()" class="btn btn-info">Restart</button>`;

  if (step == "step1") {
    $("#xyzzy2Container").html(step1);
  } else if (step == "step2") {
    $("#xyzzy2Container").html(step2);
  } else if (step == "step3") {
    soundGameOver.play();
    $("#xyzzy2Container").html(step3);
  }
}